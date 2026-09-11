#include "CampusCameraPawn.h"
#include "CampusSaveService.h"
#include "CampusConstructionService.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"
#include "Engine/GameViewportClient.h"
#include "CampusBuilding.h"
#include "CampusClock.h"
#include "EngineUtils.h"
#include "CampusHUD.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "CampusPaths.h"
#include "CampusPathService.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Widgets/SViewport.h"

ACampusCameraPawn::ACampusCameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(RootComponent);
    CameraArm->TargetArmLength = 9000.0f;
    CameraArm->SetRelativeRotation(FRotator(-55.0f, -35.0f, 0.0f));
    CameraArm->bDoCollisionTest = false;
    CameraArm->bUsePawnControlRotation = false;
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
    Camera->FieldOfView = 60.0f;
}

void ACampusCameraPawn::BeginPlay()
{
    Super::BeginPlay();
    TargetZoomDistance = FMath::Clamp(CameraArm->TargetArmLength, 2500.0f, 24000.0f);
    CameraArm->TargetArmLength = TargetZoomDistance;
    UE_LOG(LogTemp, Display, TEXT("CampusCamera: wheel zoom v2 ready (ZQSD + right-drag + wheel)."));
    // Refresh .ini bindings for a new Play session even after Live Coding.
    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
    InputSettings->ReloadConfig();
    InputSettings->ForceRebuildKeymaps();
    // The work map's PlayerStart is an old aerial placeholder. Start over the hall.
    SetActorLocationAndRotation(FVector(700.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(true);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
        {
            // Right-only capture drops a normal left press when not captured.
            // Capture on either button so the first selection click is delivered.
            Viewport->SetMouseCaptureMode(EMouseCaptureMode::CaptureDuringMouseDown);
        }
        UE_LOG(LogTemp, Display, TEXT("CampusCamera: right-drag v4 ready (direct controller mouse delta)."));
    }
}

void ACampusCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindKey(EKeys::E, IE_Pressed, this, &ACampusCameraPawn::OpenPersonnel);
    PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ACampusCameraPawn::OpenFinance);
    PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ACampusCameraPawn::HandleEscape);
    PlayerInputComponent->BindKey(EKeys::F5, IE_Pressed, this, &ACampusCameraPawn::SaveCampus);
    PlayerInputComponent->BindKey(EKeys::F9, IE_Pressed, this, &ACampusCameraPawn::LoadCampus);
    PlayerInputComponent->BindKey(EKeys::F10, IE_Pressed, this, &ACampusCameraPawn::LoadBackup);
    PlayerInputComponent->BindAxis(TEXT("CampusMoveForward"), this, &ACampusCameraPawn::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("CampusMoveRight"), this, &ACampusCameraPawn::MoveRight);
    PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACampusCameraPawn::SelectUnderCursor);
    PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ACampusCameraPawn::FinishPaths);
    PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACampusCameraPawn::ToggleSimulationPause);
    PlayerInputComponent->BindKey(EKeys::P, IE_Pressed, this, &ACampusCameraPawn::TogglePlanning);
    PlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &ACampusCameraPawn::ToggleConstruction);
    PlayerInputComponent->BindKey(EKeys::C, IE_Pressed, this, &ACampusCameraPawn::TogglePaths);
    PlayerInputComponent->BindKey(EKeys::X, IE_Pressed, this, &ACampusCameraPawn::TogglePathErase);
    PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ACampusCameraPawn::RotateConstruction);
    PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ACampusCameraPawn::SetNormalSpeed);
    PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ACampusCameraPawn::SetDoubleSpeed);
    PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ACampusCameraPawn::SetQuadrupleSpeed);
    PlayerInputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &ACampusCameraPawn::SetNormalSpeed);
    PlayerInputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &ACampusCameraPawn::SetDoubleSpeed);
    PlayerInputComponent->BindKey(EKeys::NumPadThree, IE_Pressed, this, &ACampusCameraPawn::SetQuadrupleSpeed);
    // French AZERTY number-row keys without Shift, as reported by Windows.
    PlayerInputComponent->BindKey(EKeys::Ampersand, IE_Pressed, this, &ACampusCameraPawn::SetNormalSpeed);
    PlayerInputComponent->BindKey(EKeys::E_AccentAigu, IE_Pressed, this, &ACampusCameraPawn::SetDoubleSpeed);
    PlayerInputComponent->BindKey(EKeys::Quote, IE_Pressed, this, &ACampusCameraPawn::SetQuadrupleSpeed);
    PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ACampusCameraPawn::BeginRotation);
    PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ACampusCameraPawn::EndRotation);
    // Mouse deltas are read from the controller in Tick, after input processing.
    // No new .ini axis mappings are needed during a Live Coding session.
    // A wheel notch emits a pressed/released pair; bind only pressed, once.
    PlayerInputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ACampusCameraPawn::ZoomIn);
    PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ACampusCameraPawn::ZoomOut);
}

bool ACampusCameraPawn::IsPlanningOpen() const
{
    return FCampusToolMode::IsModal(ToolMode);
}

void ACampusCameraPawn::TogglePlanning()
{
    RequestToolAction(ECampusToolAction::Planning);
}

void ACampusCameraPawn::SetNormalSpeed() { SetSimulationSpeed(1); }
void ACampusCameraPawn::SetDoubleSpeed() { SetSimulationSpeed(2); }
void ACampusCameraPawn::SetQuadrupleSpeed() { SetSimulationSpeed(4); }

void ACampusCameraPawn::SetSimulationSpeed(int32 Speed)
{
    for (TActorIterator<ACampusClock> It(GetWorld()); It; ++It)
    {
        It->SetSimulationSpeed(Speed);
        break;
    }
}

void ACampusCameraPawn::ToggleSimulationPause()
{
    for (TActorIterator<ACampusClock> It(GetWorld()); It; ++It)
    {
        It->TogglePause();
        UE_LOG(LogTemp, Display, TEXT("CampusClock: %s"), It->IsSimulationPaused() ? TEXT("paused") : TEXT("running"));
        break;
    }
}

void ACampusCameraPawn::SelectUnderCursor()
{
    if(!FSlateApplication::IsInitialized()) { HandleWorldPress(false); return; }
    auto& Slate=FSlateApplication::Get();
    HandleWorldPress(Slate.LocateWindowUnderMouse(Slate.GetCursorPos(),Slate.GetInteractiveTopLevelWindows()));
}

void ACampusCameraPawn::HandleWorldPress(const FWidgetPath& Hit)
{
    HandleWorldPress(IsTerrainHit(Hit));
}

void ACampusCameraPawn::HandleWorldPress(bool OverTerrain)
{
    if (!FCampusToolMode::AllowsWorldPress(ToolMode, OverTerrain, bIsRotatingCamera))
    { ClearPendingGesture(); return; }
    if (IsPathMode()) { ConfirmPaths(); return; }
    if (IsConstructing()) { ConfirmConstruction(); return; }
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) { return; }
    FHitResult Hit;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
    ACampusBuilding* Building = Cast<ACampusBuilding>(Hit.GetActor());
    if (SelectedBuilding.Get() == Building && (!Building || Building->IsSelected())) { return; }
    if (SelectedBuilding.IsValid()) { SelectedBuilding->SetSelected(false); }
    SelectedBuilding = Building;
    if (Building) { Building->SetSelected(true); }
    UE_LOG(LogTemp, Display, TEXT("CampusSelection: %s"), Building ? TEXT("Gymnase 01 selected") : TEXT("cleared"));
}

void ACampusCameraPawn::MoveForward(float Value)
{
    MovementInput.X = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ACampusCameraPawn::MoveRight(float Value)
{
    MovementInput.Y = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ACampusCameraPawn::BeginRotation()
{
    if (IsPlanningOpen() || !IsCursorOverTerrain()) { return; }
    // The viewport owns capture and cursor restoration. Do not switch modes
    // inside the pressed callback: that can disrupt the click's input state.
    PathAnchorX=PathAnchorY=-1;
    bIsRotatingCamera = true;
    UE_LOG(LogTemp, Display, TEXT("CampusCamera: rotation started."));
}

void ACampusCameraPawn::EndRotation()
{
    if (!bIsRotatingCamera)
    {
        return;
    }
    bIsRotatingCamera = false;
    UE_LOG(LogTemp, Display, TEXT("CampusCamera: rotation stopped."));
}

void ACampusCameraPawn::RotateHorizontal(float Value)
{
    if (bIsRotatingCamera)
    {
        FRotator Rotation = CameraArm->GetRelativeRotation();
        // Mouse input is already displacement for this frame: no DeltaSeconds.
        Rotation.Yaw = FRotator::NormalizeAxis(Rotation.Yaw + Value * 2.5f);
        CameraArm->SetRelativeRotation(Rotation);
    }
}

void ACampusCameraPawn::RotateVertical(float Value)
{
    if (bIsRotatingCamera)
    {
        FRotator Rotation = CameraArm->GetRelativeRotation();
        Rotation.Pitch = FMath::Clamp(Rotation.Pitch + Value * 2.5f, -80.0f, -25.0f);
        CameraArm->SetRelativeRotation(Rotation);
    }
}

void ACampusCameraPawn::ZoomIn()
{
    Zoom(1.0f);
}

void ACampusCameraPawn::ZoomOut()
{
    Zoom(-1.0f);
}

void ACampusCameraPawn::Zoom(float Value)
{
    if (IsPlanningOpen()) { return; }
    if (!FMath::IsNearlyZero(Value))
    {
        // Wheel up moves closer. Scale the destination so rapid notches accumulate.
        // Wheel input is an event displacement, not a rate per second.
        TargetZoomDistance = FMath::Clamp(
            TargetZoomDistance * FMath::Pow(0.85f, FMath::Clamp(Value, -10.0f, 10.0f)),
            2500.0f, 24000.0f);
        UE_LOG(LogTemp, Display, TEXT("CampusCamera: wheel %.0f, target %.0f cm"), Value, TargetZoomDistance);
    }
}

void ACampusCameraPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsLocallyControlled())
    {
        return;
    }
    if (IsPlanningOpen())
    {
        MovementInput = FVector2D::ZeroVector;
        EndRotation();
        return;
    }
    // Exponential smoothing remains consistent at different frame rates.
    const float ZoomAlpha = 1.0f - FMath::Exp(-10.0f * FMath::Max(0.0f, DeltaSeconds));
    CameraArm->TargetArmLength = FMath::Lerp(CameraArm->TargetArmLength, TargetZoomDistance, ZoomAlpha);
    if (FMath::Abs(CameraArm->TargetArmLength - TargetZoomDistance) < 0.1f)
    {
        CameraArm->TargetArmLength = TargetZoomDistance;
    }
    if (const APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        // Focus loss flushes pressed keys; never leave the cursor captured.
        if (bIsRotatingCamera && !PC->IsInputKeyDown(EKeys::RightMouseButton))
        {
            EndRotation();
        }
        if (bIsRotatingCamera)
        {
            float MouseX = 0.0f;
            float MouseY = 0.0f;
            PC->GetInputMouseDelta(MouseX, MouseY);
            RotateHorizontal(MouseX);
            RotateVertical(MouseY);
            if (!FMath::IsNearlyZero(MouseX) || !FMath::IsNearlyZero(MouseY))
            {
                const FRotator Rotation = CameraArm->GetRelativeRotation();
                UE_LOG(LogTemp, Display, TEXT("CampusCamera: orbit delta %.3f %.3f -> yaw %.2f pitch %.2f"),
                    MouseX, MouseY, Rotation.Yaw, Rotation.Pitch);
            }
        }
    }
    // Use camera yaw only: forward must never move the camera into the ground.
    const FRotator Heading(0.0f, CameraArm->GetComponentRotation().Yaw, 0.0f);
    const FVector Direction = (Heading.Vector() * MovementInput.X
        + FRotationMatrix(Heading).GetUnitAxis(EAxis::Y) * MovementInput.Y).GetClampedToMaxSize(1.0f);
    FVector Position = GetActorLocation() + Direction * MovementSpeed * DeltaSeconds;
    Position.X = FMath::Clamp(Position.X, -CampusHalfExtent, CampusHalfExtent);
    Position.Y = FMath::Clamp(Position.Y, -CampusHalfExtent, CampusHalfExtent);
    SetActorLocation(Position);
    if (IsConstructing()) { UpdateConstruction(true); }
    if (IsPathMode())
    {
        // A release consumed by a Slate panel or loss of focus must cancel, never buy later.
        auto* PC=Cast<APlayerController>(GetController());
        if(PathAnchorX>=0 && (!PC || !PC->IsInputKeyDown(EKeys::LeftMouseButton) || !IsCursorOverTerrain()))
        { PathAnchorX=PathAnchorY=-1; }
        UpdatePaths(true);
    }
}

void ACampusCameraPawn::ToggleConstruction()
{
    RequestToolAction(ECampusToolAction::Construction);
    if (IsConstructing()) { UpdateConstruction(false); }
}

void ACampusCameraPawn::RotateConstruction()
{
    if(IsPathMode()) { return; }
    if (IsConstructing() && !IsPlanningOpen())
    { ConstructionRotation = (ConstructionRotation + 1) % 4; UpdateConstruction(false); }
}

void ACampusCameraPawn::UpdateConstruction(bool bDraw)
{
    bPlacementValid = false;
    auto* PC = Cast<APlayerController>(GetController());
    FVector Origin, Direction;
    if (!IsCursorOverTerrain() || !PC || !PC->DeprojectMousePositionToWorld(Origin, Direction) || Direction.Z >= -.001)
    { ConstructionStatus = TEXT("Placez le curseur sur le terrain."); return; }
    const double Distance = -Origin.Z / Direction.Z;
    if (Distance <= 0) { ConstructionStatus = TEXT("Placez le curseur sur le terrain."); return; }
    const FVector Ground = Origin + Direction * Distance;
    Footprint = FCampusFootprint::AtCursor(Ground.X, Ground.Y, ConstructionRotation % 2 != 0);
    Footprint.X=static_cast<int>(std::floor(Footprint.X/2.0))*2;
    Footprint.Y=static_cast<int>(std::floor(Footprint.Y/2.0))*2;
    // Align the entrance edge to the broad path grid for every orientation.
    if(ConstructionRotation==2) { ++Footprint.X; }
    if(ConstructionRotation==3) { ++Footprint.Y; }
    const auto Result = FCampusConstructionService::Evaluate(GetWorld(), Footprint, ConstructionRotation);
    bPlacementValid = Result == ECampusConstructionResult::Success;
    ConstructionStatus = FCampusConstructionService::StatusText(Result);
    if (Result == ECampusConstructionResult::AlreadyBuilt || Result == ECampusConstructionResult::Unavailable) { return; }
    if (!bDraw) { return; }
    const FColor Color = bPlacementValid ? FColor(70, 225, 150) : FColor(255, 70, 60);
    for (int32 I = 0; I <= 50; ++I)
    {
        const double V = -10000 + I * 400;
        DrawDebugLine(GetWorld(), FVector(V,-10000,15), FVector(V,10000,15), FColor(80,110,85), false, -1, 0, 1);
        DrawDebugLine(GetWorld(), FVector(-10000,V,15), FVector(10000,V,15), FColor(80,110,85), false, -1, 0, 1);
    }
    const FVector Center(Footprint.CenterX(), Footprint.CenterY(), 420);
    DrawDebugBox(GetWorld(), Center, FVector(Footprint.Width()*100, Footprint.Height()*100, 400), Color, false, -1, 0, 5);
    // Ground footprint and west-side entrance turn with the building.
    DrawDebugBox(GetWorld(), FVector(Center.X, Center.Y, 25), FVector(Footprint.Width()*100, Footprint.Height()*100, 10), Color, false, -1, 0, 8);
    const FVector Entrance = FRotator(0, ConstructionRotation * 90, 0).RotateVector(FVector(-2300, 0, 40));
    DrawDebugDirectionalArrow(GetWorld(), FVector(Center.X, Center.Y, 40) + Entrance * 1.3,
        FVector(Center.X, Center.Y, 40) + Entrance * .85, 100, Color, false, -1, 0, 12);
}

void ACampusCameraPawn::ConfirmConstruction()
{
    UpdateConstruction(false); // Validate again at the click, not from last frame's preview.
    if (!bPlacementValid) { return; }
    ACampusBuilding* Built = nullptr;
    const auto Result = FCampusConstructionService::Execute(GetWorld(), Footprint, ConstructionRotation, Built);
    if (Result == ECampusConstructionResult::Success)
    { SelectedBuilding = Built; SetToolMode(ECampusToolMode::Selection); SaveStatus=TEXT("F5 : sauvegarder / F9 : charger / F10 : copie de secours"); }
    else
    { bPlacementValid = false; ConstructionStatus = FCampusConstructionService::StatusText(Result); }
}

void ACampusCameraPawn::TogglePaths()
{
    RequestToolAction(ECampusToolAction::Paths);
    if(IsPathMode()) { UpdatePaths(false); }
}
void ACampusCameraPawn::TogglePathErase()
{
    if(IsPathMode()) { bErasePaths=!bErasePaths; ClearPendingGesture(); UpdatePaths(false); }
}
void ACampusCameraPawn::UpdatePaths(bool Draw)
{
    bPathValid=false; PathX=-1; PathY=-1;
    auto* PC=Cast<APlayerController>(GetController()); FVector O,D;
    if(!IsCursorOverTerrain() || !PC || !PC->DeprojectMousePositionToWorld(O,D) || D.Z>=-.001 || O.Z<=0) { PathStatus=TEXT("Visez le terrain."); return; }
    const FVector P=O-D*(O.Z/D.Z); PathX=FCampusPathGrid::Cell(P.X); PathY=FCampusPathGrid::Cell(P.Y);
    for(TActorIterator<ACampusPaths> It(GetWorld());It;++It)
    {
        auto& G=It->GetGrid();
        const int Stride=(PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift))?1:2;
        if(!G.MakeRectangle(PathAnchorX<0?PathX:PathAnchorX,PathAnchorX<0?PathY:PathAnchorY,PathX,PathY,Stride))
        { PathStatus=TEXT("Hors parcelle."); return; }
        int32 Price=0;
        const auto Result=FCampusPathService::Evaluate(GetWorld(),bErasePaths,Price);
        bPathValid=Result==ECampusPathResult::Success;
        PathStatus=!bPathValid ? FCampusPathService::StatusText(Result)
            : FString::Printf(TEXT("%s / %d case(s) / %d EUR / %s"),bErasePaths?TEXT("SUPPRESSION sans remboursement"):TEXT("CHEMIN"),G.SegmentCount/(Stride*Stride),Price,
                PathAnchorX<0?(Stride==2?TEXT("glissez : 4 m / Maj : precision 2 m"):TEXT("glissez : precision 2 m")):TEXT("relachez : poser le rectangle"));
        if(Draw)
        {
            for(int I=0;I<=100/Stride;++I)
            {
                const double V=-10000+I*200*Stride;
                DrawDebugLine(GetWorld(),FVector(V,-10000,15),FVector(V,10000,15),FColor(80,110,85),false,-1,0,1);
                DrawDebugLine(GetWorld(),FVector(-10000,V,15),FVector(10000,V,15),FColor(80,110,85),false,-1,0,1);
            }
            for(int I=0;I<G.SegmentCount;++I)
            {
                const int C=G.Segment[I], X=C%100,Y=C/100;
                if(X%Stride || Y%Stride) { continue; }
                DrawDebugBox(GetWorld(),FVector(G.Center(X)+(Stride-1)*100,G.Center(Y)+(Stride-1)*100,20),FVector(Stride*100-2,Stride*100-2,10),bPathValid?FColor::Green:FColor::Red,false,-1,0,5);
            }
            if(It->DoorCell()!=FVector::ZeroVector) { DrawDebugBox(GetWorld(),It->DoorCell()+FVector(0,0,25),FVector(100,100,30),FColor::Cyan,false,-1,0,8); }
        }
        break;
    }
}
bool ACampusCameraPawn::IsCursorOverTerrain() const
{
    if(!FSlateApplication::IsInitialized() || !GetWorld()->GetGameViewport()) { return false; }
    auto& Slate=FSlateApplication::Get();
    const FWidgetPath Hit=Slate.LocateWindowUnderMouse(Slate.GetCursorPos(),Slate.GetInteractiveTopLevelWindows());
    return IsTerrainHit(Hit);
}
bool ACampusCameraPawn::IsTerrainHit(const FWidgetPath& Hit) const
{
    if(!GetWorld()->GetGameViewport()) { return false; }
    const auto Viewport=GetWorld()->GetGameViewport()->GetGameViewportWidget();
    return Hit.IsValid() && Viewport.IsValid() && Hit.GetLastWidget()==Viewport.ToSharedRef();
}
void ACampusCameraPawn::ConfirmPaths()
{
    PathAnchorX=PathAnchorY=-1;
    UpdatePaths(false);
    // Starting on an invalid tile is harmless: the whole rectangle is validated at release.
    if(FCampusPathGrid::Valid(PathX,PathY))
    {
        PathAnchorX=PathX; PathAnchorY=PathY;
        UE_LOG(LogTemp,Display,TEXT("CampusPaths: rectangle drag started at %d,%d"),PathX,PathY);
    }
}
void ACampusCameraPawn::FinishPaths()
{
    if(!IsPathMode() || PathAnchorX<0) { return; }
    if(IsPlanningOpen() || bIsRotatingCamera || !IsCursorOverTerrain()) { PathAnchorX=PathAnchorY=-1; return; }
    UpdatePaths(false);
    PathAnchorX=PathAnchorY=-1;
    if(!bPathValid) { return; }
    const auto Result=FCampusPathService::Execute(GetWorld(),bErasePaths);
    if(Result!=ECampusPathResult::Success) { PathStatus=FCampusPathService::StatusText(Result); }
    UE_LOG(LogTemp,Display,TEXT("CampusPaths: rectangle release, committed=%d"),Result==ECampusPathResult::Success);
}

void ACampusCameraPawn::SaveCampus() { FCampusSaveService::Save(GetWorld(), SaveStatus); }
void ACampusCameraPawn::LoadCampus() { FCampusSaveService::Load(GetWorld(), false, SaveStatus); }
void ACampusCameraPawn::LoadBackup() { FCampusSaveService::Load(GetWorld(), true, SaveStatus); }

void ACampusCameraPawn::CancelTools()
{
    RequestToolAction(ECampusToolAction::CancelTools);
}
void ACampusCameraPawn::OpenMenu()
{
    RequestToolAction(ECampusToolAction::Menu);
}
void ACampusCameraPawn::HandleEscape()
{
    RequestToolAction(ECampusToolAction::Escape);
}
void ACampusCameraPawn::ClearPendingGesture()
{
    PathAnchorX=PathAnchorY=-1;
    bPathValid=false; bPlacementValid=false;
}
void ACampusCameraPawn::RequestToolAction(ECampusToolAction Action)
{
    const auto Next=FCampusToolMode::Next(ToolMode,Action);
    if(Next!=ToolMode || Action==ECampusToolAction::CancelTools) { SetToolMode(Next); }
}
void ACampusCameraPawn::SetToolMode(ECampusToolMode Mode)
{
    ClearPendingGesture();
    MovementInput=FVector2D::ZeroVector;
    EndRotation();
    auto* PC=Cast<APlayerController>(GetController());
    auto* HUD=PC?Cast<ACampusHUD>(PC->GetHUD()):nullptr;
    // Window creation can fail (notably planning before construction).
    if(Mode!=ToolMode)
    { ToolMode=HUD && HUD->ApplyToolMode(Mode) ? Mode : ECampusToolMode::Selection; }
    // Paths require absolute cursor motion. Set this before the next mouse-down,
    // and restore hiding immediately on every exit, including modal/load flows.
    if(auto* Viewport=GetWorld()->GetGameViewport())
    { Viewport->SetHideCursorDuringCapture(!IsPathMode()); }
}
void ACampusCameraPawn::InterfaceAction(int32 Action)
{
    if(Action==0) { ToggleConstruction(); }
    else if(Action==1) { TogglePaths(); }
    else if(Action==2) { ToggleSimulationPause(); }
    else if(Action==3) { SetSimulationSpeed(1); }
    else if(Action==4) { SetSimulationSpeed(2); }
    else if(Action==5) { SetSimulationSpeed(4); }
}

void ACampusCameraPawn::OpenPersonnel()
{
    if(auto* PC=Cast<APlayerController>(GetController())) { if(auto* HUD=Cast<ACampusHUD>(PC->GetHUD())) { HUD->OpenPage(1); } }
}
void ACampusCameraPawn::OpenFinance()
{
    if(auto* PC=Cast<APlayerController>(GetController())) { if(auto* HUD=Cast<ACampusHUD>(PC->GetHUD())) { HUD->OpenPage(2); } }
}
