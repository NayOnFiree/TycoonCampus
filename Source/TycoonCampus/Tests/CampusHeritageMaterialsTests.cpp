#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Tests/AutomationCommon.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#if WITH_EDITOR
#include "AssetCompilingManager.h"
#endif
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "../CampusBuilding.h"
#include "../CampusGymGeometry.h"

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCampusArtScreenshot, FString, Filename);
bool FCampusArtScreenshot::Update()
{
#if WITH_EDITOR
    if (FAssetCompilingManager::Get().GetNumRemainingAssets() != 0)
    {
        // -game does not pump every editor asset compiler while the test is latent.
        FAssetCompilingManager::Get().FinishAllCompilation();
        return false; // Draw one complete frame before requesting the image.
    }
#endif
    UE_LOG(LogTemp, Display, TEXT("Heritage capture ready: no pending asset compilation"));
    FScreenshotRequest::RequestScreenshot(Filename, true, false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCampusHeritageMaterialsTest,
    "TycoonCampus.Runtime.HeritageMaterials",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FCampusHeritageMaterialsTest::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("CampusAutomation")))
    { AddError(TEXT("Run in the isolated automation process.")); return false; }
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
    { if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; } }
    if (!TestNotNull(TEXT("Game world"), World)) { return false; }
    auto* Building = World->SpawnActor<ACampusBuilding>();
    if (!TestNotNull(TEXT("Isolated material specimen"), Building)) { return false; }
    TArray<UStaticMeshComponent*> Components;
    Building->GetComponents(Components);
    int32 GymPartCount = 0;
    TSet<UMaterialInterface*> Finishes;
    for (auto* Mesh : Components)
    {
        const FString Name = Mesh->GetName();
        if (!Name.StartsWith(TEXT("GymPart"))) { continue; }
        ++GymPartCount;
        const int32 Index = FCString::Atoi(*Name.Mid(7));
        if (!TestTrue(TEXT("Known geometry part"), Index >= 0 && Index < UE_ARRAY_COUNT(CampusGymParts))) { continue; }
        const auto& Part = CampusGymParts[Index];
        TestTrue(TEXT("Location preserved"), Mesh->GetRelativeLocation().Equals(FVector(Part.X, Part.Y, Part.Z - 430)));
        TestTrue(TEXT("Scale preserved"), Mesh->GetRelativeScale3D().Equals(FVector(Part.SizeX, Part.SizeY, Part.SizeZ) / 100.0));
        TestTrue(TEXT("Rotation preserved"), Mesh->GetRelativeRotation().IsNearlyZero());
        TestTrue(TEXT("No decorative collision"), Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
        TestFalse(TEXT("Unbuilt gym remains hidden"), Mesh->IsVisible());
        if (Part.Material == 2)
        {
            TestEqual(TEXT("Original court marking material"), Mesh->GetMaterial(0)->GetPathName(),
                FString(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Line.M_Blockout_Line")));
            continue;
        }
        auto* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
        if (!TestNotNull(TEXT("Runtime Heritage instance assigned"), Material)) { continue; }
        Finishes.Add(Material);
        FLinearColor Color;
        float Roughness = 0;
        TestTrue(TEXT("Engine shader exposes Color"), Material->Parent->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")), Color));
        TestTrue(TEXT("Engine shader exposes Roughness"), Material->Parent->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Roughness")), Roughness));
        TestTrue(TEXT("Parent exposes Color"), Material->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")), Color));
        TestTrue(TEXT("Parent exposes Roughness"), Material->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Roughness")), Roughness));
        const bool Entry = Part.Material == 0 && Part.X == -2000;
        const FColor Expected = Index == 0 ? FColor(48,55,53) : Part.Material == 1 ? FColor(175,137,100) : Entry ? FColor(130,102,78) : FColor(200,187,164);
        TestTrue(TEXT("Palette converted from sRGB to linear"), Color.Equals(FLinearColor::FromSRGBColor(Expected), 0.0001f));
        TestTrue(TEXT("Matte finish"), Roughness >= 0.65f && Roughness <= 0.85f);
        TestTrue(TEXT("Opaque finish"), Material->GetBlendMode() == BLEND_Opaque);
    }
    TestEqual(TEXT("No geometry added or removed"), GymPartCount, static_cast<int32>(UE_ARRAY_COUNT(CampusGymParts)));
    TestEqual(TEXT("Four reused finishes"), Finishes.Num(), 4);
    auto* Bounds = Cast<UBoxComponent>(Building->GetRootComponent());
    if (TestNotNull(TEXT("Selection root retained"), Bounds))
    { TestTrue(TEXT("Selection extents retained"), Bounds->GetUnscaledBoxExtent().Equals(FVector(2100,1350,430))); }
    TestFalse(TEXT("Materials do not purchase gym"), Building->IsBuilt());
    Building->Destroy();
    // Opt-in only, in a separate rendered automation process after BootAndPanels.
    // Baseline changes material assignments on that specimen, never saved assets.
    if (FParse::Param(FCommandLine::Get(), TEXT("CampusArtCapture")))
    {
        ACampusBuilding* Built = nullptr;
        for (TActorIterator<ACampusBuilding> It(World); It; ++It)
        { if (It->IsBuilt()) { Built = *It; break; } }
        if (!TestNotNull(TEXT("Capture requires BootAndPanels construction"), Built)) { return false; }
        const bool Baseline = FParse::Param(FCommandLine::Get(), TEXT("CampusArtBaseline"));
        if (Baseline)
        {
            UMaterialInterface* Originals[] = {
                LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Wall.M_Blockout_Wall")),
                LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Court.M_Blockout_Court")),
                LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Line.M_Blockout_Line"))};
            TArray<UStaticMeshComponent*> Parts;
            Built->GetComponents(Parts);
            for (auto* Mesh : Parts)
            {
                if (!Mesh->GetName().StartsWith(TEXT("GymPart"))) { continue; }
                const int32 Index = FCString::Atoi(*Mesh->GetName().Mid(7));
                Mesh->SetMaterial(0, Originals[CampusGymParts[Index].Material]);
            }
        }
        auto* PC = World->GetFirstPlayerController();
        PC->GetPawn()->SetActorLocation(Built->GetActorLocation() - FVector(0,0,430));
        PC->ConsoleCommand(TEXT("r.EyeAdaptationQuality 0"), false);
#if WITH_EDITOR
        FAssetCompilingManager::Get().FinishAllCompilation();
#endif
        ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(15.0f));
        ADD_LATENT_AUTOMATION_COMMAND(FCampusArtScreenshot(FPaths::ProjectSavedDir() / TEXT("Art") / (Baseline ? TEXT("Baseline.png") : TEXT("Heritage.png"))));
        ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(3.0f));
    }
    return true;
}
#endif
