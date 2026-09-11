#pragma once
#include "CoreMinimal.h"

class UWorld;

enum class ECampusPathResult : uint8
{
    Success, Unavailable, InvalidOrBlocked, ProtectedPath, InsufficientFunds
};

/** Game-thread command for the path actor's current rectangle/segment selection. */
struct FCampusPathService
{
    // Read-only quote; execution always revalidates the current selection and budget.
    static ECampusPathResult Evaluate(UWorld* World, bool Erase, int32& OutPrice);
    static ECampusPathResult Execute(UWorld* World, bool Erase);
    static FString StatusText(ECampusPathResult Result);
};
