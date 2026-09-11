#include "CampusSaveService.h"
#include "CampusSaveCodec.h"
#include "CampusBuilding.h"
#include "CampusPaths.h"
#include "CampusCameraPawn.h"
#include "CampusClock.h"
#include "CampusHUD.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
    template<class T> T* Find(UWorld* W)
    { for(TActorIterator<T> It(W);It;++It) { return *It; } return nullptr; }
    FString SavePath() { return FPaths::ProjectSavedDir()/TEXT("SaveGames/CampusQuick.sav"); }
    bool Read(const FString& Path, FCampusSaveState& State)
    {
        const int64 Size=IFileManager::Get().FileSize(*Path);
        if(Size<16 || Size>static_cast<int64>(FCampusSaveCodec::MaxBytes)) { return false; }
        TArray<uint8> Bytes;
        return FFileHelper::LoadFileToArray(Bytes,*Path) && FCampusSaveCodec::Decode(Bytes.GetData(),Bytes.Num(),State);
    }
    bool Supported(UWorld* W)
    {
        if(!W) { return false; }
        FString Name=W->GetMapName(); Name.RemoveFromStart(W->StreamingLevelsPrefix);
        return Name==TEXT("L_Campus_Work");
    }
}

bool FCampusSaveService::Save(UWorld* W,FString& Status)
{
    auto* B=W ? Find<ACampusBuilding>(W) : nullptr;
    auto* P=W ? Find<ACampusPaths>(W) : nullptr;
    auto* C=W ? Find<ACampusClock>(W) : nullptr;
    auto* A=W ? Find<ACampusCameraPawn>(W) : nullptr;
    if(!Supported(W) || !B || !P || !C || !A || P->BaseBlocked.Num()!=10000)
    { Status=TEXT("Sauvegarde indisponible : lancer L_Campus_Work."); return false; }
    auto State=MakeUnique<FCampusSaveState>(); auto& S=*State;
    S.Clock=C->Time; S.Schedule=B->Schedule; S.Operations=B->Operations;
    S.Built=B->bBuilt; S.Selected=B->bIsSelected;
    S.BuildingX=B->GetActorLocation().X; S.BuildingY=B->GetActorLocation().Y;
    S.Turns=(FMath::RoundToInt(B->GetActorRotation().Yaw/90.)%4+4)%4;
    FMemory::Memcpy(S.Paths,P->GridData->Cells,sizeof(S.Paths)); S.PathReadyMinute=P->ReadyMinute;
    S.CameraX=A->GetActorLocation().X; S.CameraY=A->GetActorLocation().Y;
    S.CameraZoom=A->TargetZoomDistance; S.CameraYaw=A->CameraArm->GetRelativeRotation().Yaw;
    S.CameraPitch=A->CameraArm->GetRelativeRotation().Pitch;
    auto& V=S.Visual; V.Id=B->VisualId; V.Day=B->VisualDay; V.Started=B->bVisualStarted;
    V.People=B->VisualPeopleCount; V.Reservation=B->VisualReservation;
    V.Start=B->VisualStart; V.Arrival=B->VisualArrival; V.End=B->VisualEnd;
    V.Travel=B->VisualTravel; V.Animation=B->AnimationTime; V.RouteCount=B->VisualRoute.Num();
    if(V.RouteCount>10001) { Status=TEXT("Sauvegarde refusee : trajet invalide."); return false; }
    for(int I=0;I<V.RouteCount;++I) { const FVector Pt=B->VisualRoute[I]; V.Route[I]={Pt.X,Pt.Y,Pt.Z}; }
    TArray<uint8> Bytes; Bytes.SetNumUninitialized(FCampusSaveCodec::MaxBytes); std::size_t Size=0;
    if(!FCampusSaveCodec::Encode(S,Bytes.GetData(),Bytes.Num(),Size))
    { Status=TEXT("Sauvegarde refusee : etat incoherent. Partie conservee."); return false; }
    Bytes.SetNum(static_cast<int32>(Size));
    const FString Path=SavePath(), Temp=Path+TEXT(".tmp"), Backup=Path+TEXT(".bak");
    auto& Files=IFileManager::Get();
    auto Verified=MakeUnique<FCampusSaveState>();
    if(!Files.MakeDirectory(*FPaths::GetPath(Path),true) || !FFileHelper::SaveArrayToFile(Bytes,*Temp) || !Read(Temp,*Verified))
    { Status=TEXT("Echec d'ecriture : sauvegarde precedente conservee."); return false; }
    // Preserve the last valid main file; a corrupt main must never replace a good backup.
    if(Read(Path,*Verified))
    {
        const FString BackupTemp=Backup+TEXT(".tmp");
        if(Files.Copy(*BackupTemp,*Path,true,false,false)!=COPY_OK || !Read(BackupTemp,*Verified)
            || !Files.Move(*Backup,*BackupTemp,true,false,false,true))
        { Status=TEXT("Echec de copie de secours : sauvegarde annulee."); return false; }
    }
    if(!Files.Move(*Path,*Temp,true,false,false,true))
    { Status=TEXT("Echec de sauvegarde. Copie precedente : F10."); return false; }
    Status=TEXT("Campus sauvegarde (F5) / F9 : charger / F10 : sauvegarde precedente."); return true;
}

bool FCampusSaveService::Load(UWorld* W,bool Backup,FString& Status)
{
    auto* B=W ? Find<ACampusBuilding>(W) : nullptr;
    auto* P=W ? Find<ACampusPaths>(W) : nullptr;
    auto* C=W ? Find<ACampusClock>(W) : nullptr;
    auto* A=W ? Find<ACampusCameraPawn>(W) : nullptr;
    if(!Supported(W) || !B || !P || !C || !A || P->BaseBlocked.Num()!=10000)
    { Status=TEXT("Chargement indisponible : lancer L_Campus_Work."); return false; }
    auto State=MakeUnique<FCampusSaveState>(); auto& S=*State;
    const FString Path=SavePath()+(Backup ? TEXT(".bak") : TEXT(""));
    if(!Read(Path,S))
    { Status=TEXT("Fichier absent, incompatible ou endommage. Partie conservee. Secours : F10."); return false; }
    // Validate all map-dependent constraints before changing any actor.
    const auto F=FCampusFootprint::AtCursor(S.BuildingX,S.BuildingY,S.Turns%2!=0);
    const FVector Center(S.BuildingX,S.BuildingY,0);
    const FVector Portal=Center+FRotator(0,S.Turns*90,0).RotateVector(FVector(-2200,100,0));
    bool Valid=!S.Built || FCampusPathGrid::Valid(FCampusPathGrid::Cell(Portal.X),FCampusPathGrid::Cell(Portal.Y));
    for(int I=0;I<10000;++I)
    {
        const bool Protected=P->GridData->Cells[I]==1;
        const int X=I%100,Y=I/100;
        const bool InGym=S.Built && X>=F.X && X<F.X+F.Width() && Y>=F.Y && Y<F.Y+F.Height();
        if((S.Paths[I]==1)!=Protected || (S.Paths[I]==2 && P->BaseBlocked[I]) || (InGym && (S.Paths[I] || P->BaseBlocked[I]))) { Valid=false; }
    }
    if(!Valid) { Status=TEXT("Sauvegarde incompatible avec les obstacles de cette carte. Partie conservee."); return false; }

    if(auto* PC=Cast<APlayerController>(A->GetController()))
    { if(auto* HUD=Cast<ACampusHUD>(PC->GetHUD()); HUD && HUD->IsPlanningOpen()) { HUD->TogglePlanning(); } }
    A->EndRotation(); A->bConstructing=false; A->bPlacementValid=false; A->bPathMode=false;
    A->bPathValid=false; A->PathAnchorX=A->PathAnchorY=-1; A->MovementInput=FVector2D::ZeroVector;
    A->SetActorLocation(FVector(S.CameraX,S.CameraY,0));
    A->CameraArm->SetRelativeRotation(FRotator(S.CameraPitch,S.CameraYaw,0));
    A->TargetZoomDistance=S.CameraZoom; A->CameraArm->TargetArmLength=S.CameraZoom;
    C->Time=S.Clock; C->Time.Pause();
    B->Schedule=S.Schedule; B->Operations=S.Operations; B->bBuilt=S.Built;
    B->SetActorLocationAndRotation(Center+FVector(0,0,430),FRotator(0,S.Turns*90,0));
    B->SelectionBounds->SetCollisionEnabled(S.Built ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    for(UStaticMeshComponent* Mesh:B->GymMeshes) { Mesh->SetVisibility(S.Built); }
    B->SetSelected(S.Selected); A->SelectedBuilding=S.Selected && S.Built ? B : nullptr;
    auto& Grid=*P->GridData; FMemory::Memcpy(Grid.Cells,S.Paths,sizeof(S.Paths));
    for(int I=0;I<10000;++I) { Grid.Blocked[I]=P->BaseBlocked[I]!=0; }
    Grid.SegmentCount=0; P->Target=-1; P->Door=P->GymCenter=FVector::ZeroVector; P->bConnected=false;
    if(S.Built) { P->SetGym(Center,S.Turns,C->GetTotalMinutes()); } else { P->Refresh(C->GetTotalMinutes()); }
    P->ReadyMinute=S.PathReadyMinute; B->bConnected=P->bConnected; B->RouteMetres=P->DistanceMetres();
    auto& V=S.Visual; B->VisualId=V.Id; B->VisualDay=V.Day; B->bVisualStarted=V.Started;
    B->VisualPeopleCount=V.People; B->VisualReservation=V.Reservation;
    B->VisualStart=V.Start; B->VisualArrival=V.Arrival; B->VisualEnd=V.End; B->VisualTravel=V.Travel; B->AnimationTime=V.Animation;
    B->VisualRoute.Reset(); for(int I=0;I<V.RouteCount;++I) { B->VisualRoute.Add(FVector(V.Route[I].X,V.Route[I].Y,V.Route[I].Z)); }
    B->VisiblePeople=0;
    B->FigureBodies->SetVisibility(false); B->FigureHeads->SetVisibility(false); B->FigureLimbs->SetVisibility(false);
    if(V.Id) { B->FigureBodies->SetMaterial(0,B->FigureMaterials[static_cast<int>(V.Reservation.Activity)]); }
    if(S.Built) { B->UpdateFigures(C->GetTotalSeconds(),0,true,P); }
    Status=Backup ? TEXT("Copie de secours chargee EN PAUSE. Espace : reprendre.") : TEXT("Campus charge EN PAUSE. Espace : reprendre / F5 : sauvegarder.");
    return true;
}
