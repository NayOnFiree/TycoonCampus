#include "CampusConstructionService.h"
#include "CampusBuilding.h"
#include "CampusClock.h"
#include "CampusPaths.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

namespace
{
    // This prototype supports one gym. Ambiguous worlds must not debit an arbitrary actor.
    template<class T> T* FindUnique(UWorld* World)
    {
        T* Found = nullptr;
        for (TActorIterator<T> It(World); It; ++It)
        { if (Found) { return nullptr; } Found = *It; }
        return Found;
    }
}

ECampusConstructionResult FCampusConstructionService::Evaluate(UWorld* World,
    const FCampusFootprint& Footprint, int32 QuarterTurns)
{
    using R = ECampusConstructionResult;
    if (!IsInGameThread() || !World) { return R::Unavailable; }
    auto* Building = FindUnique<ACampusBuilding>(World);
    auto* Paths = FindUnique<ACampusPaths>(World);
    auto* Clock = FindUnique<ACampusClock>(World);
    if (!Building || !Paths || !Clock || !Paths->HasActorBegunPlay()) { return R::Unavailable; }
    if (Building->IsBuilt()) { return R::AlreadyBuilt; }
    if (QuarterTurns < 0 || QuarterTurns > 3 || Footprint.Rotated != (QuarterTurns % 2 != 0))
    { return R::InvalidRequest; }
    if (!Footprint.InsideParcel()) { return R::OutsideParcel; }
    const FVector Center(Footprint.CenterX(), Footprint.CenterY(), 0);
    const FVector Portal = Center + FRotator(0, QuarterTurns * 90, 0).RotateVector(FVector(-2200, 100, 0));
    if (!FCampusPathGrid::Valid(FCampusPathGrid::Cell(Portal.X), FCampusPathGrid::Cell(Portal.Y)))
    { return R::EntranceOutsideParcel; }
    if (Paths->HasPathIn(Footprint)) { return R::Occupied; }
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        if (It->IsHidden()) { continue; }
        FVector BoundsCenter, Extent;
        It->GetActorBounds(false, BoundsCenter, Extent);
        if (BoundsCenter.Z + Extent.Z <= 1 || BoundsCenter.Z - Extent.Z > 850) { continue; }
        if (Footprint.Overlaps(BoundsCenter.X - Extent.X, BoundsCenter.Y - Extent.Y,
            BoundsCenter.X + Extent.X, BoundsCenter.Y + Extent.Y)) { return R::Occupied; }
    }
    return Building->GetOperations().Cash < FCampusOperations::GymCost ? R::InsufficientFunds : R::Success;
}

ECampusConstructionResult FCampusConstructionService::Execute(UWorld* World,
    const FCampusFootprint& Footprint, int32 QuarterTurns, ACampusBuilding*& OutBuilding)
{
    OutBuilding = nullptr;
    const auto Result = Evaluate(World, Footprint, QuarterTurns);
    if (Result != ECampusConstructionResult::Success) { return Result; }
    auto* Building = FindUnique<ACampusBuilding>(World);
    auto* Clock = FindUnique<ACampusClock>(World);
    if (!Building || !Clock || !Building->ApplyConstruction(
        FVector(Footprint.CenterX(), Footprint.CenterY(), 0), QuarterTurns, Clock->GetTotalMinutes()))
    { return ECampusConstructionResult::Unavailable; }
    OutBuilding = Building;
    return ECampusConstructionResult::Success;
}

FString FCampusConstructionService::StatusText(ECampusConstructionResult Result)
{
    switch (Result)
    {
    case ECampusConstructionResult::Success: return FString::Printf(TEXT("Emplacement valide - clic gauche : construire pour %lld EUR."), FCampusOperations::GymCost);
    case ECampusConstructionResult::AlreadyBuilt: return TEXT("Gymnase deja construit - un seul dans ce prototype. B : quitter.");
    case ECampusConstructionResult::InvalidRequest: return TEXT("Placement invalide : orientation incoherente.");
    case ECampusConstructionResult::OutsideParcel: return TEXT("Hors parcelle : toute l'empreinte doit tenir dans le terrain.");
    case ECampusConstructionResult::EntranceOutsideParcel: return TEXT("L'entree doit pouvoir etre raccordee dans la parcelle. Tournez le gymnase.");
    case ECampusConstructionResult::Occupied: return TEXT("Emplacement occupe : chemin, entree ou obstacle.");
    case ECampusConstructionResult::InsufficientFunds: return FString::Printf(TEXT("Fonds insuffisants : %lld EUR necessaires."), FCampusOperations::GymCost);
    default: return TEXT("Gymnase indisponible.");
    }
}
