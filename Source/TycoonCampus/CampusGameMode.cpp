#include "CampusGameMode.h"
#include "CampusCameraPawn.h"
#include "CampusBuilding.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "CampusClock.h"
#include "CampusHUD.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "CampusGymGeometry.h"
#include "CampusPaths.h"

ACampusGameMode::ACampusGameMode()
{
    DefaultPawnClass = ACampusCameraPawn::StaticClass();
}

void ACampusGameMode::BeginPlay()
{
    Super::BeginPlay();
    // Hide only the original blockout parts in this Play world; never edit the saved map.
    for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
    {
        for (const auto& Part : CampusGymParts)
        {
            if (It->GetActorLocation().Equals(FVector(Part.X + 700, Part.Y, Part.Z), .5)
                && It->GetActorScale3D().Equals(FVector(Part.SizeX, Part.SizeY, Part.SizeZ) / 100.0, .005)
                && It->GetActorRotation().IsNearlyZero())
            { It->SetActorHiddenInGame(true); It->SetActorEnableCollision(false); break; }
        }
    }
    if (!TActorIterator<ACampusClock>(GetWorld()))
    {
        GetWorld()->SpawnActor<ACampusClock>();
    }
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->ClientSetHUD(ACampusHUD::StaticClass());
    }
    if (!TActorIterator<ACampusPaths>(GetWorld())) { GetWorld()->SpawnActor<ACampusPaths>(); }
    // The unbuilt hall holds the prototype's budget until the first placement.
    if (!TActorIterator<ACampusBuilding>(GetWorld()))
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        GetWorld()->SpawnActor<ACampusBuilding>(FVector(700, 0, 430), FRotator::ZeroRotator, Params);
    }
}
