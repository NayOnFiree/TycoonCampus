#include "CampusPathService.h"
#include "CampusBuilding.h"
#include "CampusClock.h"
#include "CampusPaths.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    template<class T> T* FindUnique(UWorld* World)
    {
        T* Found = nullptr;
        for (TActorIterator<T> It(World); It; ++It)
        { if (Found) { return nullptr; } Found = *It; }
        return Found;
    }
}

ECampusPathResult FCampusPathService::Evaluate(UWorld* World, bool Erase, int32& OutPrice)
{
    using R = ECampusPathResult;
    OutPrice = 0;
    if (!IsInGameThread() || !World) { return R::Unavailable; }
    auto* Building = FindUnique<ACampusBuilding>(World);
    auto* Paths = FindUnique<ACampusPaths>(World);
    auto* Clock = FindUnique<ACampusClock>(World);
    if (!Building || !Paths || !Clock || !Paths->HasActorBegunPlay()) { return R::Unavailable; }
    const int Price = Paths->GetGrid().Quote(Erase);
    if (Price == -2) { return R::ProtectedPath; }
    if (Price < 0) { return R::InvalidOrBlocked; }
    OutPrice = Price;
    return !Erase && Building->GetOperations().Cash < Price ? R::InsufficientFunds : R::Success;
}

ECampusPathResult FCampusPathService::Execute(UWorld* World, bool Erase)
{
    int32 Price = 0;
    const auto Result = Evaluate(World, Erase, Price);
    if (Result != ECampusPathResult::Success) { return Result; }
    auto* Paths = FindUnique<ACampusPaths>(World);
    auto& Operations = FindUnique<ACampusBuilding>(World)->GetOperations();
    const int64 Minutes = FindUnique<ACampusClock>(World)->GetTotalMinutes();
    // No asynchronous work between validation, debit, ledger and world refresh.
    if (!Paths->GetGrid().Apply(Erase, Operations.Cash)) { return ECampusPathResult::Unavailable; }
    if (Price > 0) { Operations.RecordPathInvestment(Price, Minutes); }
    Paths->Refresh(Minutes);
    return ECampusPathResult::Success;
}

FString FCampusPathService::StatusText(ECampusPathResult Result)
{
    switch (Result)
    {
    case ECampusPathResult::ProtectedPath: return TEXT("Chemin initial protege : impossible de le supprimer.");
    case ECampusPathResult::InvalidOrBlocked: return TEXT("Obstacle ou gymnase : trace refuse.");
    case ECampusPathResult::InsufficientFunds: return TEXT("Fonds insuffisants pour ce trace.");
    case ECampusPathResult::Success: return TEXT("");
    default: return TEXT("Chemins indisponibles.");
    }
}
