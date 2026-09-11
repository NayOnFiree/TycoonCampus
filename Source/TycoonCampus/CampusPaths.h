#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampusPathGrid.h"
#include "CampusConstruction.h"
#include "CampusPaths.generated.h"
class UInstancedStaticMeshComponent;

UCLASS()
class TYCOONCAMPUS_API ACampusPaths : public AActor
{
    GENERATED_BODY()
    friend struct FCampusSaveService;
public:
    ACampusPaths();
    virtual void BeginPlay() override;
    FCampusPathGrid& GetGrid() { return *GridData; }
    bool HasPathIn(const FCampusFootprint& F) const;
    void SetGym(const FVector& Center, int32 Turns, int64 Minutes);
    void Refresh(int64 Minutes);
    bool IsConnected() const { return bConnected; }
    double DistanceMetres() const { return RouteLength / 100.; }
    double ReadyMinute = 0;
    FVector DoorCell() const { return Door; }
    FVector PointOnRoute(double Fraction) const;
    const TArray<FVector>& RoutePoints() const { return Points; }
private:
    TUniquePtr<FCampusPathGrid> GridData = MakeUnique<FCampusPathGrid>();
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Mesh;
    TArray<uint8> BaseBlocked;
    FVector Door = FVector::ZeroVector;
    FVector GymCenter = FVector::ZeroVector;
    int Target = -1;
    bool bConnected = false;
    TArray<FVector> Points;
    double RouteLength=0;
};
