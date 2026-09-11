#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampusWeeklySchedule.h"
#include "CampusOperations.h"
#include "CampusBuilding.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class ACampusPaths;
class UMaterialInterface;

/** Logical selection target for the existing hall blockout. */
UCLASS()
class TYCOONCAMPUS_API ACampusBuilding : public AActor
{
    GENERATED_BODY()
    friend struct FCampusSaveService;
    friend struct FCampusConstructionService;
public:
    ACampusBuilding();
    virtual void Tick(float DeltaSeconds) override;
    bool IsBuilt() const { return bBuilt; }
    bool IsConnected() const { return bConnected; }
    double GetRouteMetres() const { return RouteMetres; }
    int32 GetVisiblePeople() const { return VisiblePeople; }
    FString GetActivityStatus() const;
    FCampusOperations& GetOperations() { return Operations; }
    const FCampusOperations& GetOperations() const { return Operations; }
    void SetSelected(bool bSelected);
    bool IsSelected() const { return bIsSelected; }
    FCampusWeeklySchedule& GetSchedule() { return Schedule; }
    const FCampusWeeklySchedule& GetSchedule() const { return Schedule; }

private:
    virtual void BeginPlay() override;
    void ApplyHeritageMaterials();
    UPROPERTY()
    TObjectPtr<UMaterialInterface> HeritageSurface;

    // Only the validated game command can purchase and materialize the gym.
    bool ApplyConstruction(const FVector& GroundCenter, int32 QuarterTurns, int64 Minutes);
    UPROPERTY(VisibleAnywhere, Category="Campus")
    TObjectPtr<UBoxComponent> SelectionBounds;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> SelectionFrame;
    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> GymMeshes;
    bool bBuilt = false;

    bool bIsSelected = false;
    FCampusWeeklySchedule Schedule;
    FCampusOperations Operations;
    bool bConnected=false;
    double RouteMetres=0;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> FigureBodies;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> FigureHeads;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> FigureLimbs;
    UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> FigureMaterials;
    int32 VisiblePeople=0;
    int32 VisualPeopleCount=8;
    bool bVisualStarted=false;
    int64 VisualId=0, VisualDay=-1;
    FCampusReservation VisualReservation{};
    TArray<FVector> VisualRoute;
    double VisualStart=0, VisualArrival=0, VisualEnd=0, VisualTravel=0, AnimationTime=0;
    void UpdateFigures(double Seconds, float DeltaSeconds, bool Paused, ACampusPaths* Paths);
};
