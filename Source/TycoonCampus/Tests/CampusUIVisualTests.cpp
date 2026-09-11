#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "ImageUtils.h"
#include "ShaderCompiler.h"
#include "Layout/Children.h"
#include "UnrealClient.h"
#include "Widgets/SWindow.h"
#include "../CampusBuilding.h"
#include "../CampusClock.h"
#include "../CampusConstructionService.h"
#include "../CampusHUD.h"

namespace
{
    TSharedPtr<SWidget> FindPlanning(const TSharedRef<SWidget>& Widget)
    {
        if (Widget->GetTypeAsString() == TEXT("SCampusPlanningPanel")) { return Widget; }
        FChildren* Children = Widget->GetChildren();
        for (int32 Index = 0; Index < Children->Num(); ++Index)
        {
            if (auto Found = FindPlanning(Children->GetChildAt(Index))) { return Found; }
        }
        return nullptr;
    }

    // One command owns the capture delegate and restores its transient scale on every exit.
    class FCaptureCampusUI : public IAutomationLatentCommand
    {
    public:
        FCaptureCampusUI(FAutomationTestBase* InTest, ACampusHUD* InHUD, float InScale,
            int32 InWidth, int32 InHeight, const FString& InOutput)
            : Test(InTest), HUD(InHUD), Scale(InScale), Width(InWidth), Height(InHeight), Output(InOutput)
        {
            PreviousScale = GetDefault<UUserInterfaceSettings>()->ApplicationScale;
            auto* Settings = GetMutableDefault<UUserInterfaceSettings>();
            Settings->ApplicationScale = 1.f;
            BaselineUIScale = Settings->GetDPIScaleBasedOnSize(FIntPoint(Width, Height));
            // Normalize the project's resolution curve so 1.25 means an effective UI scale
            // of 1.25, not 0.666 * 1.25 at 720p. This is explicitly NOT an OS DPI change.
            Settings->ApplicationScale = Scale / BaselineUIScale;
            PreviousTooltips = FSlateApplication::Get().GetAllowTooltips();
            FSlateApplication::Get().SetAllowTooltips(false);
            StageStarted = FPlatformTime::Seconds();
        }
        ~FCaptureCampusUI()
        {
            UGameViewportClient::OnScreenshotCaptured().Remove(CaptureHandle);
            GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = PreviousScale;
            if (FSlateApplication::IsInitialized()) { FSlateApplication::Get().SetAllowTooltips(PreviousTooltips); }
        }
        bool Update() override
        {
            if (!HUD.IsValid() || !GEngine || !GEngine->GameViewport)
            { Test->AddError(TEXT("Visual fixture lost its viewport/HUD")); return true; }
            const double Elapsed = FPlatformTime::Seconds() - StageStarted;
            if (Elapsed > 180.) { Test->AddError(TEXT("Timed out waiting for shaders / a rendered UI screenshot")); return true; }
            if (bRequested)
            {
                if (!bCaptured) { return false; }
                UGameViewportClient::OnScreenshotCaptured().Remove(CaptureHandle);
                if (View == 1) { return true; }
                HUD->TogglePlanning();
                if (!HUD->IsPlanningOpen()) { Test->AddError(TEXT("Planning did not open")); return true; }
                View = 1;
                bRequested = bCaptured = false;
                StageStarted = FPlatformTime::Seconds();
                return false;
            }
            // Let Slate geometry, streaming and the real renderer settle across frames.
            if (Elapsed < 5.) { return false; }
            if (GShaderCompilingManager && GShaderCompilingManager->IsCompiling()) { return false; }
            FVector2D Size;
            GEngine->GameViewport->GetViewportSize(Size);
            if (FMath::RoundToInt(Size.X) != Width || FMath::RoundToInt(Size.Y) != Height)
            {
                Test->AddError(FString::Printf(TEXT("Viewport is %.0fx%.0f, requested %dx%d"), Size.X, Size.Y, Width, Height));
                return true;
            }
            const auto Window = GEngine->GameViewport->GetWindow();
            if (!Window.IsValid()) { Test->AddError(TEXT("No Slate window for UI capture")); return true; }
            auto Planning = FindPlanning(Window.ToSharedRef());
            if (View == 1 && !Planning.IsValid())
            { Test->AddError(TEXT("Planning widget missing from Slate tree")); return true; }
            WindowDPI = Window->GetDPIScaleFactor();
            SlateApplicationScale = FSlateApplication::Get().GetApplicationScale();
            EffectiveUIScale = GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(Width, Height));
            Test->TestTrue(TEXT("Requested effective UI scale applied"), FMath::IsNearlyEqual(EffectiveUIScale, Scale, 0.001f));
            if (Planning.IsValid())
            {
                PlanningSize = Planning->GetCachedGeometry().GetLocalSize();
                PlanningScale = Planning->GetCachedGeometry().GetAccumulatedLayoutTransform().GetScale();
                Test->TestTrue(TEXT("Planning geometry uses requested effective scale"), FMath::IsNearlyEqual(PlanningScale, Scale, 0.01f));
            }
            CaptureHandle = UGameViewportClient::OnScreenshotCaptured().AddRaw(this, &FCaptureCampusUI::Captured);
            bRequested = true;
            FScreenshotRequest::RequestScreenshot(Output / Stem(), true, false, false, FIntRect(), true);
            return false;
        }
    private:
        FString Stem() const
        {
            return FString::Printf(TEXT("%dx%d_ui%03d_%s"), Width, Height, FMath::RoundToInt(Scale * 100),
                View == 0 ? TEXT("hud") : TEXT("planning"));
        }
        void Captured(int32 PixelWidth, int32 PixelHeight, const TArray<FColor>& Pixels)
        {
            bCaptured = true;
            Test->TestEqual(TEXT("Capture width"), PixelWidth, Width);
            Test->TestEqual(TEXT("Capture height"), PixelHeight, Height);
            bool NonUniform = false;
            for (int32 Index = 1; Index < Pixels.Num(); ++Index)
            { if (Pixels[Index] != Pixels[0]) { NonUniform = true; break; } }
            Test->TestTrue(TEXT("Capture contains nonuniform rendered pixels"), NonUniform);
            TArray64<uint8> PNG;
            FImageUtils::PNGCompressImageArray(PixelWidth, PixelHeight, TArrayView64<const FColor>(Pixels.GetData(), Pixels.Num()), PNG);
            Test->TestTrue(TEXT("PNG written"), FFileHelper::SaveArrayToFile(PNG, *(Output / (Stem() + TEXT(".png")))));
            const FString Metadata = FString::Printf(TEXT(
                "{\n  \"schemaVersion\":1,\n  \"view\":\"%s\",\n  \"requestedWidth\":%d,\n  \"requestedHeight\":%d,\n"
                "  \"captureWidth\":%d,\n  \"captureHeight\":%d,\n  \"requestedEffectiveUIScale\":%.4f,\n"
                "  \"baselineResolutionUIScale\":%.4f,\n  \"unrealUIApplicationScale\":%.4f,\n"
                "  \"effectiveViewportUIScale\":%.4f,\n  \"slateApplicationScale\":%.4f,\n  \"windowDPIScale\":%.4f,\n"
                "  \"planningAllottedWidth\":%.4f,\n  \"planningAllottedHeight\":%.4f,\n  \"planningGeometryScale\":%.4f,\n"
                "  \"realRHI\":true,\n  \"includesUI\":true,\n  \"tooltipsSuppressed\":true,\n  \"windowsDPIChanged\":false,\n"
                "  \"fixture\":\"fresh isolated world; paused; built selected gym; empty recurring schedule\",\n"
                "  \"scope\":\"Unreal UI scale stress, not Windows DPI emulation; no real input gesture validation\"\n}\n"),
                View == 0 ? TEXT("hud") : TEXT("planning"), Width, Height, PixelWidth, PixelHeight,
                Scale, BaselineUIScale, GetDefault<UUserInterfaceSettings>()->ApplicationScale,
                EffectiveUIScale, SlateApplicationScale, WindowDPI, PlanningSize.X, PlanningSize.Y, PlanningScale);
            Test->TestTrue(TEXT("Metadata written"), FFileHelper::SaveStringToFile(Metadata, *(Output / (Stem() + TEXT(".json"))), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM));
        }
        FAutomationTestBase* Test;
        TWeakObjectPtr<ACampusHUD> HUD;
        float Scale, PreviousScale = 1, BaselineUIScale = 1, WindowDPI = 1, SlateApplicationScale = 1, EffectiveUIScale = 1, PlanningScale = 0;
        int32 Width, Height, View = 0;
        FString Output;
        FVector2D PlanningSize = FVector2D::ZeroVector;
        double StageStarted = 0;
        bool bRequested = false, bCaptured = false;
        bool PreviousTooltips = true;
        FDelegateHandle CaptureHandle;
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCampusUIVisualTest, "TycoonCampus.Visual.CaptureUI",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FCampusUIVisualTest::RunTest(const FString& Parameters)
{
    FString Output;
    int32 Width = 0, Height = 0;
    float Scale = 0;
    if (!FParse::Param(FCommandLine::Get(), TEXT("CampusVisualAutomation"))
        || FParse::Param(FCommandLine::Get(), TEXT("nullrhi")) || !FApp::CanEverRender()
        || !FParse::Value(FCommandLine::Get(), TEXT("CampusCaptureDir="), Output)
        || !FParse::Value(FCommandLine::Get(), TEXT("ResX="), Width)
        || !FParse::Value(FCommandLine::Get(), TEXT("ResY="), Height)
        || !FParse::Value(FCommandLine::Get(), TEXT("CampusVisualScale="), Scale))
    { AddError(TEXT("Use Scripts/Capture-UI.ps1 in an isolated project with a real RHI")); return false; }
    if (!((Width == 1280 && Height == 720) || (Width == 1920 && Height == 1080))
        || !(FMath::IsNearlyEqual(Scale, 1.f) || FMath::IsNearlyEqual(Scale, 1.25f) || FMath::IsNearlyEqual(Scale, 1.5f)))
    { AddError(TEXT("Unsupported matrix case")); return false; }
    FString UserDirectory;
    if (!FParse::Value(FCommandLine::Get(), TEXT("UserDir="), UserDirectory) || UserDirectory.IsEmpty()
        || !IFileManager::Get().FileExists(*(FPaths::ProjectDir() / TEXT("CampusVisualFixture.txt"))))
    { AddError(TEXT("A temporary UserDir is required")); return false; }
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
    { if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; } }
    if (!TestNotNull(TEXT("Isolated game world"), World)) { return false; }
    auto* PC = World->GetFirstPlayerController();
    auto* HUD = PC ? Cast<ACampusHUD>(PC->GetHUD()) : nullptr;
    if (!TestNotNull(TEXT("Campus HUD"), HUD)) { return false; }
    TActorIterator<ACampusBuilding> Buildings(World);
    ACampusBuilding* Building = Buildings ? *Buildings : nullptr;
    if (!TestNotNull(TEXT("Gym fixture"), Building) || !TestFalse(TEXT("Fresh unbuilt world"), Building->IsBuilt())) { return false; }
    for (TActorIterator<ACampusClock> It(World); It; ++It)
    { if (!It->IsSimulationPaused()) { It->TogglePause(); } }
    ACampusBuilding* Built = nullptr;
    for (int32 Y = 30; Y <= 86 && !Built; Y += 2)
    {
        for (int32 X = 30; X <= 78 && !Built; X += 2)
        {
            FCampusFootprint Candidate{X, Y, false};
            if (FCampusConstructionService::Evaluate(World, Candidate, 0) == ECampusConstructionResult::Success)
            { FCampusConstructionService::Execute(World, Candidate, 0, Built); }
        }
    }
    if (!TestNotNull(TEXT("Gym built through construction command"), Built)) { return false; }
    Built->SetSelected(true);
    if (APawn* Camera = PC->GetPawn())
    {
        const FVector Center = Built->GetActorLocation();
        Camera->SetActorLocation(FVector(Center.X, Center.Y, Camera->GetActorLocation().Z));
    }
    IFileManager::Get().MakeDirectory(*Output, true);
    ADD_LATENT_AUTOMATION_COMMAND(FCaptureCampusUI(this, HUD, Scale, Width, Height, Output));
    return true;
}
#endif
