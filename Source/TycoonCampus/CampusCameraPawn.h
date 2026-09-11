#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CampusConstruction.h"
#include "CampusCameraPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ACampusBuilding;

/** AZERTY movement, right-mouse orbit and smooth wheel zoom around a ground anchor. */
UCLASS()
class TYCOONCAMPUS_API ACampusCameraPawn : public APawn
{

    GENERATED_BODY()
    friend struct FCampusSaveService;

public:
    ACampusCameraPawn();
    void SaveCampus();
    void InterfaceAction(int32 Action);
    void OpenMenu();
    void OpenPersonnel();
    void OpenFinance();
    void CancelTools();
    void LoadCampus();
    void LoadBackup();
    FString SaveStatus = TEXT("F5 : sauvegarder / F9 : charger / F10 : copie de secours");
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    bool IsConstructing() const { return bConstructing; }
    bool IsPathMode() const { return bPathMode; }
    FString GetPathStatus() const { return PathStatus; }
    FString GetConstructionStatus() const { return ConstructionStatus; }

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campus|Camera")
    TObjectPtr<USpringArmComponent> CameraArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Campus|Camera")
    TObjectPtr<UCameraComponent> Camera;

    /** Centimetres per second. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campus|Camera", meta=(ClampMin="0"))
    float MovementSpeed = 2200.0f;

    /** Limits the ground point the camera looks at, not the lens position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Campus|Camera", meta=(ClampMin="0"))
    float CampusHalfExtent = 9000.0f;

private:
    void MoveForward(float Value);
    void ToggleConstruction();
    void TogglePaths();
    void TogglePathErase();
    void UpdatePaths(bool Draw);
    void ConfirmPaths();
    void FinishPaths();
    bool IsCursorOverTerrain() const;
    bool bPathMode=false, bErasePaths=false, bVerticalPath=false, bPathValid=false;
    int32 PathAnchorX=-1, PathAnchorY=-1, PathX=-1, PathY=-1;
    FString PathStatus;
    void RotateConstruction();
    void UpdateConstruction(bool bDraw);
    void ConfirmConstruction();
    bool bConstructing = false;
    bool bPlacementValid = false;
    int32 ConstructionRotation = 0;
    FCampusFootprint Footprint;
    FString ConstructionStatus;
    void MoveRight(float Value);
    void BeginRotation();
    void EndRotation();
    void RotateHorizontal(float Value);
    void RotateVertical(float Value);
    void Zoom(float Value);
    void ZoomIn();
    void ZoomOut();
    void SelectUnderCursor();
    void ToggleSimulationPause();
    void TogglePlanning();
    bool IsPlanningOpen() const;
    void SetNormalSpeed();
    void SetDoubleSpeed();
    void SetQuadrupleSpeed();
    void SetSimulationSpeed(int32 Speed);
    TWeakObjectPtr<ACampusBuilding> SelectedBuilding;
    float TargetZoomDistance = 9000.0f;
    bool bIsRotatingCamera = false;
    FVector2D CursorBeforeRotation = FVector2D::ZeroVector;
    FVector2D MovementInput = FVector2D::ZeroVector;
};
