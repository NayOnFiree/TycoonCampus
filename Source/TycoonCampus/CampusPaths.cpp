#include "CampusPaths.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"

ACampusPaths::ACampusPaths()
{
    Mesh=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PathTiles")); RootComponent=Mesh;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Path.M_Blockout_Path"));
    Mesh->SetStaticMesh(Cube.Object); Mesh->SetMaterial(0,Material.Object); Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ACampusPaths::BeginPlay()
{
    Super::BeginPlay();
    auto& Grid=*GridData;
    // Import the existing entrance plaza and approach as protected public paths.
    for(int Y=0;Y<100;++Y) for(int X=0;X<100;++X)
    {
        const double WX=Grid.Center(X), WY=Grid.Center(Y);
        if((WX>=-9100 && WX<=-7900 && WY>=-800 && WY<=800) || (WX>=-7900 && WX<=-1200 && WY>=-200 && WY<=200))
        { Grid.Cells[Y*100+X]=1; }
    }
    for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It)
    {
        if(It->IsHidden()) { continue; }
        FVector C,E; It->GetActorBounds(false,C,E);
        if(C.Z+E.Z<=15 || C.Z-E.Z>850) { continue; }
        for(int Y=0;Y<100;++Y) for(int X=0;X<100;++X)
        {
            if(Grid.Center(X)-100<C.X+E.X && Grid.Center(X)+100>C.X-E.X && Grid.Center(Y)-100<C.Y+E.Y && Grid.Center(Y)+100>C.Y-E.Y)
            { Grid.Blocked[Y*100+X]=true; }
        }
    }
    BaseBlocked.SetNum(10000);
    for(int I=0;I<10000;++I) { BaseBlocked[I]=Grid.Blocked[I]; }
}
bool ACampusPaths::HasPathIn(const FCampusFootprint& F) const
{
    const auto& Grid=*GridData;
    if(!F.InsideParcel()) { return true; }
    for(int Y=F.Y;Y<F.Y+F.Height();++Y) for(int X=F.X;X<F.X+F.Width();++X) { if(Grid.Cells[Y*100+X]) { return true; } }
    return false;
}
void ACampusPaths::SetGym(const FVector& Center,int32 Turns,int64 Minutes)
{
    auto& Grid=*GridData;
    GymCenter=Center+FVector(0,0,30);
    const auto F=FCampusFootprint::AtCursor(Center.X,Center.Y,Turns%2!=0);
    for(int Y=F.Y;Y<F.Y+F.Height();++Y) for(int X=F.X;X<F.X+F.Width();++X) { Grid.Blocked[Y*100+X]=true; }
    const FVector Portal=Center+FRotator(0,Turns*90,0).RotateVector(FVector(-2200,100,0));
    const int X=Grid.Cell(Portal.X), Y=Grid.Cell(Portal.Y);
    Target=Grid.Valid(X,Y) ? Y*100+X : -1;
    Door=FVector(Grid.Center(X),Grid.Center(Y),20);
    Refresh(Minutes);
}
void ACampusPaths::Refresh(int64 Minutes)
{
    auto& Grid=*GridData;
    Mesh->ClearInstances();
    for(int I=0;I<10000;++I) if(Grid.Cells[I]==2)
    { Mesh->AddInstance(FTransform(FRotator::ZeroRotator,FVector(Grid.Center(I%100),Grid.Center(I/100),6),FVector(2,2,.12))); }
    const bool WasConnected=bConnected;
    bConnected=Grid.FindRoute(50*100+7,Target);
    Points.Reset(); RouteLength=0;
    if(bConnected)
    {
        if(!WasConnected) { ReadyMinute=static_cast<double>(Minutes); }
        for(int I=0;I<Grid.RouteCount;++I)
        {
            const int C=Grid.Route[I]; Points.Add(FVector(Grid.Center(C%100),Grid.Center(C/100),20));
        }
        Points.Add(GymCenter);
        for(int I=1;I<Points.Num();++I) { RouteLength+=FVector::Distance(Points[I-1],Points[I]); }
    }
}
FVector ACampusPaths::PointOnRoute(double Fraction) const
{
    if(Points.IsEmpty()) { return Door; }
    double Remaining=FMath::Clamp(Fraction,0.,1.)*RouteLength;
    for(int I=1;I<Points.Num();++I)
    {
        const double Length=FVector::Distance(Points[I-1],Points[I]);
        if(Remaining<=Length) { return FMath::Lerp(Points[I-1],Points[I],Length>0 ? Remaining/Length : 0.); }
        Remaining-=Length;
    }
    return Points.Last();
}
