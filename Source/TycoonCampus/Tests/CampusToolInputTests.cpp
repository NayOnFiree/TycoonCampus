#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/ScopeExit.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Components/InputComponent.h"
#include "InputKeyEventArgs.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Widgets/SViewport.h"
#include "Widgets/Layout/SBorder.h"
#include "../CampusCameraPawn.h"
#include "../CampusHUD.h"
#include "../CampusBuilding.h"
#include "../CampusPaths.h"
#include "../CampusConstructionService.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCampusToolInputTest,
    "TycoonCampus.Runtime.ToolInput",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FCampusToolInputTest::RunTest(const FString& Parameters)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("CampusAutomation")))
    { AddError(TEXT("Run through Scripts/Test-Unreal.ps1 in an isolated process.")); return false; }
    UWorld* World=nullptr;
    for(const auto& Context:GEngine->GetWorldContexts())
    { if(Context.WorldType==EWorldType::Game) { World=Context.World(); break; } }
    if(!TestNotNull(TEXT("Game world"),World)) { return false; }
    auto* PC=World->GetFirstPlayerController();
    auto* Pawn=PC?Cast<ACampusCameraPawn>(PC->GetPawn()):nullptr;
    auto* HUD=PC?Cast<ACampusHUD>(PC->GetHUD()):nullptr;
    auto* Viewport=World->GetGameViewport();
    if(!TestNotNull(TEXT("Pawn"),Pawn) || !TestNotNull(TEXT("HUD"),HUD)
        || !TestNotNull(TEXT("Viewport"),Viewport) || !TestNotNull(TEXT("PlayerInput"),PC->PlayerInput.Get())
        || !TestNotNull(TEXT("Bound input component"),Pawn->InputComponent.Get())) { return false; }
    TActorIterator<ACampusBuilding> BuildingIt(World);
    TActorIterator<ACampusPaths> PathsIt(World);
    if(!BuildingIt || !PathsIt) { AddError(TEXT("Campus actors missing")); return false; }
    auto* Building=*BuildingIt;
    auto& Grid=PathsIt->GetGrid();
    // Also runnable alone: set up a gym via the real construction command.
    if(!Building->IsBuilt())
    {
        ACampusBuilding* Built=nullptr;
        for(int Y=0;Y<=86 && !Built;Y+=2) for(int X=0;X<=78 && !Built;X+=2)
        { FCampusConstructionService::Execute(World,{X,Y,false},0,Built); }
        if(!TestNotNull(TEXT("Constructed fixture"),Built)) { return false; }
    }
    using M=ECampusToolMode;
    Pawn->SetToolMode(M::Selection);
    auto& Slate=FSlateApplication::Get();
    const TArray<UInputComponent*> Stack{Pawn->InputComponent.Get()};
    ON_SCOPE_EXIT
    {
        PC->PlayerInput->FlushPressedKeys();
        Pawn->SetToolMode(M::Selection);
        Slate.SetAllUserFocusToGameViewport();
    };
    auto Pump=[&]() { PC->PlayerInput->ProcessInputStack(Stack,1.f/60.f,false); };
    auto GameKey=[&](FKey Key)
    {
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Pressed,1.f)); Pump();
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key,IE_Released,0.f)); Pump();
    };
    auto SlateKey=[&](FKey Key,bool Repeat=false)
    {
        const FKeyEvent Event(Key,FModifierKeysState(),0,Repeat,0,0);
        TestTrue(TEXT("Focused Slate key consumed"),Slate.ProcessKeyDownEvent(Event));
        Slate.ProcessKeyUpEvent(Event); Pump();
    };
    auto ModeIs=[&](const TCHAR* Label,M Mode)
    {
        TestTrue(Label,Pawn->GetToolMode()==Mode);
        TestEqual(TEXT("Exclusive construction state"),Pawn->IsConstructing(),Mode==M::Construction);
        TestEqual(TEXT("Exclusive paths state"),Pawn->IsPathMode(),Mode==M::Paths);
        TestEqual(TEXT("Planning window follows input"),HUD->IsPlanningOpen(),Mode==M::Planning);
        TestEqual(TEXT("Menu follows input"),HUD->IsMenuOpen(),Mode==M::Menu);
        TestEqual(TEXT("Capture hiding follows tool"),Viewport->HideCursorDuringCapture(),Mode!=M::Paths);
    };
    GameKey(EKeys::B); ModeIs(TEXT("B binding enters construction"),M::Construction);
    GameKey(EKeys::C); ModeIs(TEXT("C binding replaces construction"),M::Paths);
    GameKey(EKeys::Escape); ModeIs(TEXT("Escape binding cancels paths"),M::Selection);
    Slate.SetAllUserFocusToGameViewport();
    SlateKey(EKeys::Escape); ModeIs(TEXT("Slate Escape opens menu"),M::Menu);
    SlateKey(EKeys::Escape,true); ModeIs(TEXT("Held Escape does not close menu"),M::Menu);
    SlateKey(EKeys::Escape); ModeIs(TEXT("Slate Escape closes menu"),M::Selection);
    GameKey(EKeys::B); GameKey(EKeys::P); ModeIs(TEXT("P binding replaces construction with planning"),M::Planning);
    SlateKey(EKeys::P); ModeIs(TEXT("P closes focused planning through Slate"),M::Selection);
    GameKey(EKeys::C); GameKey(EKeys::P); ModeIs(TEXT("P replaces paths"),M::Planning);
    SlateKey(EKeys::Escape); ModeIs(TEXT("Slate Escape closes planning"),M::Selection);

    // Exercise the production widget-path boundary with a real viewport overlay.
    // No native mouse movement or geometric hit-test is claimed in NullRHI.
    const auto Panel=SNew(SBorder).OnMouseButtonDown_Lambda(
        [](const FGeometry&,const FPointerEvent&) { return FReply::Handled(); });
    Viewport->AddViewportWidgetContent(Panel,100);
    ON_SCOPE_EXIT { Viewport->RemoveViewportWidgetContent(Panel); };
    FWidgetPath PanelPath, TerrainPath;
    const auto Terrain=Viewport->GetGameViewportWidget();
    if(!Terrain.IsValid() || !TestTrue(TEXT("Actual panel has a Slate widget path"),Slate.GeneratePathToWidgetUnchecked(Panel,PanelPath))
        || !TestTrue(TEXT("Viewport has a Slate widget path"),Slate.GeneratePathToWidgetUnchecked(Terrain.ToSharedRef(),TerrainPath))) { return false; }
    TestTrue(TEXT("Viewport path classified as terrain"),Pawn->IsTerrainHit(TerrainPath));
    TestFalse(TEXT("Panel path is not terrain"),Pawn->IsTerrainHit(PanelPath));
    TestFalse(TEXT("Missing path cannot interact with world"),Pawn->IsTerrainHit(FWidgetPath()));
    Pawn->SelectedBuilding=Building; Building->SetSelected(true);
    const auto BeforeGrid=MakeUnique<FCampusPathGrid>(Grid);
    auto& Operations=Building->GetOperations();
    const auto BeforeOperations=MakeUnique<FCampusOperations>(Operations);
    for(auto Mode:{M::Selection,M::Construction,M::Paths,M::Planning,M::Menu})
    {
        Pawn->SetToolMode(Mode);
        Pawn->PathAnchorX=20; Pawn->PathAnchorY=20;
        Pawn->bPathValid=true; Pawn->bPlacementValid=true;
        Pawn->HandleWorldPress(PanelPath);
        PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::LeftMouseButton,IE_Released,0.f)); Pump();
        ModeIs(TEXT("Panel click does not switch tools"),Mode);
        TestEqual(TEXT("Panel click cancels pending drag"),Pawn->PathAnchorX,-1);
        TestFalse(TEXT("Panel click invalidates placement"),Pawn->bPlacementValid);
        TestTrue(TEXT("Panel click preserves selected actor"),Pawn->SelectedBuilding.Get()==Building && Building->IsSelected());
        TestTrue(TEXT("Panel click/release preserves grid"),FMemory::Memcmp(BeforeGrid.Get(),&Grid,sizeof(Grid))==0);
        TestTrue(TEXT("Panel click/release preserves budget and ledger"),FMemory::Memcmp(BeforeOperations.Get(),&Operations,sizeof(Operations))==0);
    }
    return true;
}
#endif
