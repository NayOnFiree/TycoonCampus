#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "../CampusGameMode.h"
#include "../CampusBuilding.h"
#include "../CampusPaths.h"
#include "../CampusClock.h"
#include "../CampusCameraPawn.h"
#include "../CampusHUD.h"
#include "../CampusConstruction.h"
#include "../CampusConstructionService.h"
#include "../CampusPathService.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCampusBootAndPanelsTest,
    "TycoonCampus.Runtime.BootAndPanels",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FCampusBootAndPanelsTest::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("CampusAutomation")))
    { AddError(TEXT("Use the isolated Scripts/Test-Unreal.ps1 process.")); return false; }
    UWorld* World = nullptr;
    for (const auto& Context : GEngine->GetWorldContexts())
    { if (Context.WorldType == EWorldType::Game) { World = Context.World(); break; } }
    if (!TestNotNull(TEXT("Game world (run with -game)"), World)) { return false; }
    TestTrue(TEXT("Work map loaded"), World->GetMapName().EndsWith(TEXT("L_Campus_Work")));
    TestNotNull(TEXT("Campus game mode"), Cast<ACampusGameMode>(World->GetAuthGameMode()));
    int32 Buildings = 0, Paths = 0, Clocks = 0;
    for (TActorIterator<ACampusBuilding> It(World); It; ++It) { ++Buildings; TestFalse(TEXT("Fresh campus unbuilt"), It->IsBuilt()); }
    for (TActorIterator<ACampusPaths> It(World); It; ++It) { ++Paths; }
    for (TActorIterator<ACampusClock> It(World); It; ++It) { ++Clocks; TestTrue(TEXT("Starts paused"), It->IsSimulationPaused()); }
    TestEqual(TEXT("One building model"), Buildings, 1);
    TestEqual(TEXT("One path system"), Paths, 1);
    TestEqual(TEXT("One clock"), Clocks, 1);
    auto* PC = World->GetFirstPlayerController();
    if (!TestNotNull(TEXT("Player controller"), PC)) { return false; }
    TestNotNull(TEXT("Camera possessed"), Cast<ACampusCameraPawn>(PC->GetPawn()));
    auto* HUD = Cast<ACampusHUD>(PC->GetHUD());
    if (!TestNotNull(TEXT("Campus HUD"), HUD)) { return false; }
    TestFalse(TEXT("No initial modal"), HUD->IsModalOpen());
    HUD->ToggleMenu(); TestTrue(TEXT("Menu opens"), HUD->IsMenuOpen());
    HUD->ToggleMenu(); TestFalse(TEXT("Menu closes"), HUD->IsModalOpen());
    HUD->TogglePlanning(); TestFalse(TEXT("Planning blocked before construction"), HUD->IsPlanningOpen());
    TActorIterator<ACampusBuilding> BuildingIt(World);
    ACampusBuilding* Building = BuildingIt ? *BuildingIt : nullptr;
    if (!TestNotNull(TEXT("Building available"), Building)) { return false; }
    using R = ECampusConstructionResult;
    FCampusFootprint Footprint;
    bool FoundPlacement = false;
    for (int Y = 0; Y <= 86 && !FoundPlacement; Y += 2)
    {
        for (int X = 0; X <= 78; X += 2)
        {
            const FCampusFootprint Candidate{X, Y, false};
            if (FCampusConstructionService::Evaluate(World, Candidate, 0) == R::Success)
            { Footprint = Candidate; FoundPlacement = true; break; }
        }
    }
    if (!TestTrue(TEXT("Map has a legal gym placement"), FoundPlacement)) { return false; }
    ACampusBuilding* Constructed = Building;
    auto& Operations = Building->GetOperations();
    auto* Pawn = Cast<ACampusCameraPawn>(PC->GetPawn());
    using M = ECampusToolMode;
    auto ModeIs = [&](const TCHAR* Label, M Mode)
    {
        TestTrue(Label, Pawn->GetToolMode()==Mode);
        TestEqual(TEXT("Construction view matches mode"), Pawn->IsConstructing(), Mode==M::Construction);
        TestEqual(TEXT("Path view matches mode"), Pawn->IsPathMode(), Mode==M::Paths);
        TestEqual(TEXT("Planning view matches mode"), HUD->IsPlanningOpen(), Mode==M::Planning);
        TestEqual(TEXT("Menu view matches mode"), HUD->IsMenuOpen(), Mode==M::Menu);
    };
    Pawn->InterfaceAction(0); ModeIs(TEXT("Construction from UI"), M::Construction);
    Pawn->InterfaceAction(1); ModeIs(TEXT("Paths replace construction"), M::Paths);
    Pawn->PathAnchorX=4; Pawn->PathAnchorY=4; Pawn->bPathValid=true;
    Pawn->HandleEscape(); ModeIs(TEXT("Escape cancels tool without opening menu"), M::Selection);
    TestEqual(TEXT("Escape cancels pending anchor"), Pawn->PathAnchorX, -1);
    TestFalse(TEXT("Escape invalidates pending quote"), Pawn->bPathValid);
    Pawn->HandleEscape(); ModeIs(TEXT("Second escape opens menu"), M::Menu);
    Pawn->InterfaceAction(0); ModeIs(TEXT("Construction blocked by menu"), M::Menu);
    HUD->HandleEscape(); ModeIs(TEXT("HUD escape uses same transition"), M::Selection);
    Pawn->InterfaceAction(1);
    HUD->TogglePlanning(); ModeIs(TEXT("Unavailable planning returns to selection"), M::Selection);
    TestFalse(TEXT("Unavailable planning leaves no modal"), HUD->IsModalOpen());
    const FVector InitialLocation = Building->GetActorLocation();
    auto Reject = [&](const TCHAR* Label, const FCampusFootprint& Request, int Turns, R Expected)
    {
        const auto Cash = Operations.Cash;
        Constructed = Building;
        TestTrue(Label, FCampusConstructionService::Execute(World, Request, Turns, Constructed) == Expected);
        TestNull(TEXT("Rejected command has no output building"), Constructed);
        TestFalse(TEXT("Rejected command leaves gym unbuilt"), Building->IsBuilt());
        TestEqual(TEXT("Rejected command preserves cash"), Operations.Cash, Cash);
        TestEqual(TEXT("Rejected command preserves ledger"), Operations.EntryCount(), 0);
        TestEqual(TEXT("Rejected command preserves investment"), Operations.Investment, static_cast<std::int64_t>(0));
        TestTrue(TEXT("Rejected command preserves transform"), Building->GetActorLocation().Equals(InitialLocation));
    };
    Reject(TEXT("Outside parcel rejected"), {-1, 20, false}, 0, R::OutsideParcel);
    Reject(TEXT("Entrance outside parcel rejected"), {0, 20, false}, 0, R::EntranceOutsideParcel);
    Reject(TEXT("Invalid orientation rejected"), Footprint, 4, R::InvalidRequest);
    Reject(TEXT("Mismatched footprint rejected"), Footprint, 1, R::InvalidRequest);
    auto* PathActor = *TActorIterator<ACampusPaths>(World);
    const int Cell = Footprint.Y * 100 + Footprint.X;
    auto& Grid = PathActor->GetGrid();
    const auto PreviousCell = Grid.Cells[Cell];
    Grid.Cells[Cell] = 2; // A path appeared since the successful preview.
    Reject(TEXT("New path invalidates preview"), Footprint, 0, R::Occupied);
    Grid.Cells[Cell] = PreviousCell;
    Operations.Cash = 59999;
    Reject(TEXT("Budget change invalidates preview"), Footprint, 0, R::InsufficientFunds);
    Operations.Cash = 100000;
    TestTrue(TEXT("Construct gym through shared command"),
        FCampusConstructionService::Execute(World, Footprint, 0, Constructed) == R::Success);
    TestTrue(TEXT("Command returns constructed building"), Constructed == Building);
    TestEqual(TEXT("Construction debited"), Building->GetOperations().Cash, static_cast<std::int64_t>(40000));
    TestTrue(TEXT("Repeated command rejected"),
        FCampusConstructionService::Execute(World, Footprint, 0, Constructed) == R::AlreadyBuilt);
    TestNull(TEXT("Repeated command clears output"), Constructed);
    TestEqual(TEXT("Repeated command never debits twice"), Operations.Cash, static_cast<std::int64_t>(40000));
    TestEqual(TEXT("Exactly one purchase in ledger"), Operations.EntryCount(), 1);
    using P = ECampusPathResult;
    int FreeCell = -1, ProtectedCell = -1;
    for (int I = 0; I < 9999; ++I)
    {
        if (Grid.Cells[I] == 1) { ProtectedCell = I; }
        if (I % 100 < 99 && !Grid.Cells[I] && !Grid.Cells[I+1] && !Grid.Blocked[I] && !Grid.Blocked[I+1])
        { FreeCell = I; }
    }
    if (!TestTrue(TEXT("Map has free pair and protected path"), FreeCell >= 0 && ProtectedCell >= 0)) { return false; }
    auto* PathMesh = PathActor->FindComponentByClass<UInstancedStaticMeshComponent>();
    if (!TestNotNull(TEXT("Path instances"), PathMesh)) { return false; }
    auto SelectPair = [&]() { Grid.MakeRectangle(FreeCell%100, FreeCell/100, FreeCell%100+1, FreeCell/100, 1); };
    auto Unchanged = [&](const TCHAR* Label, bool Erase, P Expected)
    {
        // Full model snapshots include ledger contents, route and the pending selection.
        const auto BeforeGrid = MakeUnique<FCampusPathGrid>(Grid);
        const auto BeforeOperations = MakeUnique<FCampusOperations>(Operations);
        const auto Points = PathActor->RoutePoints();
        const auto Connected = PathActor->IsConnected();
        const auto Ready = PathActor->ReadyMinute;
        const auto Distance = PathActor->DistanceMetres();
        const auto Instances = PathMesh->GetInstanceCount();
        const auto Transform = Building->GetActorTransform();
        int32 Quote = -1;
        TestTrue(TEXT("Preview and execution agree"), FCampusPathService::Evaluate(World, Erase, Quote) == Expected);
        TestTrue(Label, FCampusPathService::Execute(World, Erase) == Expected);
        TestTrue(TEXT("Grid including route and selection unchanged"), FMemory::Memcmp(BeforeGrid.Get(), &Grid, sizeof(Grid)) == 0);
        TestTrue(TEXT("Budget, investments and full ledger unchanged"), FMemory::Memcmp(BeforeOperations.Get(), &Operations, sizeof(Operations)) == 0);
        TestTrue(TEXT("World route unchanged"), Points == PathActor->RoutePoints());
        TestEqual(TEXT("Connectivity unchanged"), PathActor->IsConnected(), Connected);
        TestEqual(TEXT("Access time unchanged"), PathActor->ReadyMinute, Ready);
        TestEqual(TEXT("Route distance unchanged"), PathActor->DistanceMetres(), Distance);
        TestEqual(TEXT("Path instances unchanged"), PathMesh->GetInstanceCount(), Instances);
        TestTrue(TEXT("Building transform unchanged"), Building->GetActorTransform().Equals(Transform));
    };
    SelectPair();
    int32 Quote = 0;
    TestTrue(TEXT("Free pair preview accepted"), FCampusPathService::Evaluate(World, false, Quote) == P::Success);
    TestEqual(TEXT("Legacy cells cost 20 each"), Quote, 40);
    const auto Budget = Operations.Cash;
    Operations.Cash = 39;
    Unchanged(TEXT("Budget changed after preview"), false, P::InsufficientFunds);
    Operations.Cash = Budget;
    Grid.Blocked[FreeCell+1] = true;
    Unchanged(TEXT("Obstacle changed after preview rejects whole pair"), false, P::InvalidOrBlocked);
    Grid.Blocked[FreeCell+1] = false;
    const auto EntryCount = Operations.EntryCount();
    const auto Investment = Operations.Investment;
    const auto InstanceCount = PathMesh->GetInstanceCount();
    TestTrue(TEXT("Buy selected pair"), FCampusPathService::Execute(World, false) == P::Success);
    TestEqual(TEXT("Exact debit"), Operations.Cash, Budget-40);
    TestEqual(TEXT("One investment entry"), Operations.EntryCount(), EntryCount+1);
    TestEqual(TEXT("Investment total"), Operations.Investment, Investment+40);
    TestTrue(TEXT("Both cells purchased"), Grid.Cells[FreeCell] == 2 && Grid.Cells[FreeCell+1] == 2);
    TestEqual(TEXT("World instances refreshed"), PathMesh->GetInstanceCount(), InstanceCount+2);
    Unchanged(TEXT("Repeated purchase succeeds without extra debit"), false, P::Success);
    // Mixed selection must not erase the purchased cell before finding protection.
    Grid.Segment[1] = ProtectedCell;
    Unchanged(TEXT("Protected path rejects whole removal"), true, P::ProtectedPath);
    SelectPair();
    Operations.Cash = -1; // Removal remains available even with a negative balance.
    TestTrue(TEXT("Remove pair"), FCampusPathService::Execute(World, true) == P::Success);
    TestEqual(TEXT("Removal never refunds"), Operations.Cash, static_cast<std::int64_t>(-1));
    TestEqual(TEXT("Removal does not journal a purchase"), Operations.EntryCount(), EntryCount+1);
    TestTrue(TEXT("Both cells removed"), Grid.Cells[FreeCell] == 0 && Grid.Cells[FreeCell+1] == 0);
    TestEqual(TEXT("Removed instances refreshed"), PathMesh->GetInstanceCount(), InstanceCount);
    Unchanged(TEXT("Repeated removal is harmless"), true, P::Success);
    Operations.Cash = Budget-40;
    Grid.SegmentCount = 0;
    Unchanged(TEXT("Empty selection refused"), false, P::InvalidOrBlocked);
    Quote = 123;
    TestTrue(TEXT("Missing world unavailable"), FCampusPathService::Evaluate(nullptr, false, Quote) == P::Unavailable);
    TestEqual(TEXT("Unavailable quote cleared"), Quote, 0);
    HUD->TogglePlanning(); TestTrue(TEXT("Planning opens"), HUD->IsPlanningOpen());
    HUD->TogglePlanning(); TestFalse(TEXT("Planning closes"), HUD->IsModalOpen());
    Pawn->InterfaceAction(1);
    Pawn->PathAnchorX=FreeCell%100; Pawn->PathAnchorY=FreeCell/100;
    HUD->OpenPage(1); ModeIs(TEXT("Personnel replaces paths with planning"), M::Planning);
    TestEqual(TEXT("Opening modal cancels anchor"), Pawn->PathAnchorX, -1);
    Pawn->InterfaceAction(1); ModeIs(TEXT("Paths blocked by planning"), M::Planning);
    HUD->HandleEscape(); ModeIs(TEXT("Escape closes planning without menu"), M::Selection);
    HUD->OpenPage(2); ModeIs(TEXT("Finance uses planning mode"), M::Planning);
    Pawn->OpenMenu(); ModeIs(TEXT("Explicit menu replaces planning"), M::Menu);
    Pawn->CancelTools(); ModeIs(TEXT("Load cleanup retains menu"), M::Menu);
    Pawn->HandleEscape(); ModeIs(TEXT("Menu escape returns to selection"), M::Selection);
    TestTrue(TEXT("Closing menu remains paused"), (*TActorIterator<ACampusClock>(World))->IsSimulationPaused());

    const auto BeforeInputGrid=MakeUnique<FCampusPathGrid>(Grid);
    const auto BeforeInputOperations=MakeUnique<FCampusOperations>(Operations);
    Pawn->SelectedBuilding=Building;
    Building->SetSelected(true);
    // Feed the UI hit classification into the exact handler used by SelectUnderCursor.
    // The Slate hit test and real mouse gestures still require an interactive check.
    for(auto Mode : {M::Selection,M::Construction,M::Paths,M::Planning,M::Menu})
    {
        Pawn->SetToolMode(Mode);
        Pawn->PathAnchorX=FreeCell%100; Pawn->PathAnchorY=FreeCell/100;
        Pawn->bPathValid=true; Pawn->bPlacementValid=true;
        Pawn->HandleWorldPress(false);
        Pawn->FinishPaths(); // A later release must not revive the canceled gesture.
        TestEqual(TEXT("UI press cancels pending path"), Pawn->PathAnchorX, -1);
        TestFalse(TEXT("UI press clears path validity"), Pawn->bPathValid);
        TestFalse(TEXT("UI press clears construction validity"), Pawn->bPlacementValid);
        TestTrue(TEXT("UI press preserves selection"), Pawn->SelectedBuilding.Get()==Building && Building->IsSelected());
        TestTrue(TEXT("UI press/release preserves grid"), FMemory::Memcmp(BeforeInputGrid.Get(), &Grid, sizeof(Grid))==0);
        TestTrue(TEXT("UI press/release preserves economy and ledger"), FMemory::Memcmp(BeforeInputOperations.Get(), &Operations, sizeof(Operations))==0);
        ModeIs(TEXT("UI press does not change mode"),Mode);
    }
    Pawn->SetToolMode(M::Paths);
    Pawn->PathAnchorX=3; Pawn->PathAnchorY=4; Pawn->bIsRotatingCamera=true;
    Pawn->InterfaceAction(0); ModeIs(TEXT("Construction cancels path gesture and rotation"), M::Construction);
    TestEqual(TEXT("Transition clears both anchors"), Pawn->PathAnchorY, -1);
    TestFalse(TEXT("Transition ends rotation"), Pawn->bIsRotatingCamera);
    Pawn->CancelTools(); ModeIs(TEXT("Cleanup returns to selection"), M::Selection);
    return true;
}
#endif
