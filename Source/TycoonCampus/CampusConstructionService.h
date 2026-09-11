#pragma once
#include "CoreMinimal.h"
#include "CampusConstruction.h"

class UWorld;
class ACampusBuilding;

enum class ECampusConstructionResult : uint8
{
    Success, Unavailable, AlreadyBuilt, InvalidRequest, OutsideParcel,
    EntranceOutsideParcel, Occupied, InsufficientFunds
};

/** Game-thread command shared by placement previews, player confirmation and automation. */
struct FCampusConstructionService
{
    static ECampusConstructionResult Evaluate(UWorld* World, const FCampusFootprint& Footprint, int32 QuarterTurns);
    // Re-evaluates the current world and budget. OutBuilding is set only on success.
    static ECampusConstructionResult Execute(UWorld* World, const FCampusFootprint& Footprint,
        int32 QuarterTurns, ACampusBuilding*& OutBuilding);
    static FString StatusText(ECampusConstructionResult Result);
};
