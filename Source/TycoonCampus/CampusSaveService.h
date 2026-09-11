#pragma once
#include "CoreMinimal.h"
class UWorld;
struct FCampusSaveService
{
    static bool Save(UWorld* World, FString& Status);
    static bool Load(UWorld* World, bool Backup, FString& Status);
};
