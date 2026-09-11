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
    HUD->TogglePlanning(); TestTrue(TEXT("Planning opens"), HUD->IsPlanningOpen());
    HUD->TogglePlanning(); TestFalse(TEXT("Planning closes"), HUD->IsModalOpen());
    return true;
}
#endif
