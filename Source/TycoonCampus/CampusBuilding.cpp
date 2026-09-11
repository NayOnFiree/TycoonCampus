#include "CampusBuilding.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "CampusClock.h"
#include "EngineUtils.h"
#include "CampusGymGeometry.h"
#include "CampusPaths.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"

ACampusBuilding::ACampusBuilding()
{
    PrimaryActorTick.bCanEverTick = true;
    SelectionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SelectionBounds"));
    RootComponent = SelectionBounds;
    // Enclose the complete 40 x 26 m blockout, including walls, not just its floor.
    SelectionBounds->SetBoxExtent(FVector(2100.0f, 1350.0f, 430.0f));
    SelectionBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SelectionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    SelectionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    SelectionBounds->SetGenerateOverlapEvents(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Surface(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    HeritageSurface = Surface.Object;
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Selection.M_Selection"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Wall(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Wall.M_Blockout_Wall"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Court(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Court.M_Blockout_Court"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Line(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Line.M_Blockout_Line"));
    UMaterialInterface* Materials[] = {Wall.Object, Court.Object, Line.Object};
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Grass(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Grass.M_Blockout_Grass"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PathMat(TEXT("/Game/TycoonCampus/Blockout/Materials/M_Blockout_Path.M_Blockout_Path"));
    FigureMaterials={Wall.Object,Court.Object,Grass.Object,Line.Object,PathMat.Object};
    FigureBodies=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RepresentativeBodies"));
    FigureHeads=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RepresentativeHeads"));
    FigureLimbs=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RepresentativeLimbs"));
    UInstancedStaticMeshComponent* Figures[]={FigureBodies,FigureHeads,FigureLimbs};
    UStaticMesh* FigureShapes[]={Cylinder.Object,Sphere.Object,Cube.Object};
    for(int I=0;I<3;++I)
    {
        Figures[I]->SetupAttachment(RootComponent); Figures[I]->SetStaticMesh(FigureShapes[I]); Figures[I]->SetMaterial(0,Wall.Object);
        Figures[I]->SetCollisionEnabled(ECollisionEnabled::NoCollision); Figures[I]->SetVisibility(false);
    }
    int32 PartIndex = 0;
    for (const auto& Part : CampusGymParts)
    {
        auto* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(*FString::Printf(TEXT("GymPart%d"), PartIndex++)));
        Mesh->SetupAttachment(RootComponent);
        Mesh->SetStaticMesh(Cube.Object);
        Mesh->SetMaterial(0, Materials[Part.Material]);
        Mesh->SetRelativeLocation(FVector(Part.X, Part.Y, Part.Z - 430));
        Mesh->SetRelativeScale3D(FVector(Part.SizeX, Part.SizeY, Part.SizeZ) / 100.0);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetVisibility(false);
        GymMeshes.Add(Mesh);
    }
    const FVector Positions[] = {
        FVector(-1925, -1400, -395), FVector(1925, -1400, -395),
        FVector(-1925, 1400, -395), FVector(1925, 1400, -395),
        FVector(-2150, -1175, -395), FVector(-2150, 1175, -395),
        FVector(2150, -1175, -395), FVector(2150, 1175, -395)};
    const FVector Scales[] = {
        FVector(4.5f, .18f, .06f), FVector(4.5f, .18f, .06f),
        FVector(4.5f, .18f, .06f), FVector(4.5f, .18f, .06f),
        FVector(.18f, 4.5f, .06f), FVector(.18f, 4.5f, .06f),
        FVector(.18f, 4.5f, .06f), FVector(.18f, 4.5f, .06f)};
    for (int32 Index = 0; Index < 8; ++Index)
    {
        UStaticMeshComponent* Edge = CreateDefaultSubobject<UStaticMeshComponent>(FName(*FString::Printf(TEXT("SelectionEdge%d"), Index)));
        Edge->SetupAttachment(RootComponent);
        Edge->SetStaticMesh(Cube.Object);
        if (Material.Succeeded()) { Edge->SetMaterial(0, Material.Object); }
        Edge->SetRelativeLocation(Positions[Index]);
        Edge->SetRelativeScale3D(Scales[Index]);
        Edge->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Edge->SetCastShadow(false);
        Edge->SetVisibility(false);
        SelectionFrame.Add(Edge);
    }
}

void ACampusBuilding::BeginPlay()
{
    Super::BeginPlay();
    ApplyHeritageMaterials();
}

void ACampusBuilding::ApplyHeritageMaterials()
{
    if (!ensureMsgf(HeritageSurface, TEXT("Heritage requires the engine BasicShapeMaterial"))) { return; }
    auto MakeSurface = [this](const TCHAR* Name, FColor SRGB, float Roughness)
    {
        auto* Instance = UMaterialInstanceDynamic::Create(HeritageSurface, this, FName(Name));
        Instance->SetVectorParameterValue(TEXT("Color"), FLinearColor::FromSRGBColor(SRGB));
        Instance->SetScalarParameterValue(TEXT("Roughness"), Roughness);
        return Instance;
    };
    auto* Stone = MakeSurface(TEXT("HeritageStone"), FColor(200, 187, 164), 0.85f);
    auto* Tobacco = MakeSurface(TEXT("HeritageTobacco"), FColor(130, 102, 78), 0.75f);
    auto* SportsWood = MakeSurface(TEXT("HeritageSportsWood"), FColor(175, 137, 100), 0.65f);
    // A matte painted finish on the existing thin base; no exposed metallic reflection.
    auto* DarkMetal = MakeSurface(TEXT("HeritageDarkMetal"), FColor(48, 55, 53), 0.8f);
    for (int32 Index = 0; Index < GymMeshes.Num(); ++Index)
    {
        const auto& Part = CampusGymParts[Index];
        if (Part.Material == 2) { continue; } // Keep the original court markings.
        UMaterialInterface* Finish = Part.Material == 1 ? SportsWood : Stone;
        if (Index == 0) { Finish = DarkMetal; } // Existing floor slab, unchanged geometry.
        else if (Part.Material == 0 && Part.X == -2000) { Finish = Tobacco; }
        GymMeshes[Index]->SetMaterial(0, Finish);
    }
}

void ACampusBuilding::SetSelected(bool bSelected)
{
    bSelected = bSelected && bBuilt;
    bIsSelected = bSelected;
    for (UStaticMeshComponent* Edge : SelectionFrame)
    {
        Edge->SetVisibility(bSelected);
    }
}

void ACampusBuilding::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bBuilt) { return; }
    ACampusPaths* Paths=nullptr;
    for(TActorIterator<ACampusPaths> It(GetWorld());It;++It) { Paths=*It; break; }
    bConnected=Paths && Paths->IsConnected(); RouteMetres=Paths ? Paths->DistanceMetres() : 0;
    if(Paths && !bConnected) { DrawDebugBox(GetWorld(),Paths->DoorCell()+FVector(0,0,25),FVector(100,100,30),FColor::Cyan,false,-1,0,6); }
    Operations.SetAccess(bConnected,RouteMetres,Paths ? Paths->ReadyMinute : 0);
    for (TActorIterator<ACampusClock> It(GetWorld()); It; ++It)
    {
        if (!It->IsSimulationPaused()) { Operations.AdvanceTo(It->GetTotalMinutes() / 60, Schedule); }
        UpdateFigures(It->GetTotalSeconds(),DeltaSeconds,It->IsSimulationPaused(),Paths);
        break;
    }
}

bool ACampusBuilding::ApplyConstruction(const FVector& GroundCenter, int32 QuarterTurns, int64 Minutes)
{
    if (bBuilt || !Operations.PurchaseGym(Minutes)) { return false; }
    SetActorLocationAndRotation(GroundCenter + FVector(0, 0, 430), FRotator(0, QuarterTurns * 90, 0));
    bBuilt = true;
    SelectionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    for (UStaticMeshComponent* Mesh : GymMeshes) { Mesh->SetVisibility(true); }
    for(TActorIterator<ACampusPaths> It(GetWorld());It;++It) { It->SetGym(GroundCenter,QuarterTurns,Minutes); break; }
    SetSelected(true);
    return true;
}

FString ACampusBuilding::GetActivityStatus() const
{
    if(!bBuilt) { return TEXT("Gymnase non construit"); }
    if(!bConnected) { return TEXT("NON RACCORDE - seances futures impossibles"); }
    const TCHAR* Names[]={TEXT("Universite"),TEXT("Entrainement"),TEXT("Acces libre"),TEXT("Cours public"),TEXT("Location club")};
    if(Operations.IsActive())
    {
        if(!Operations.ActiveCanRun()) { return TEXT("Seance bloquee - motif au journal a sa fin"); }
        return FString::Printf(TEXT("%s / %d participants simules / %d figurants / retard %d min"),
            Names[static_cast<int>(Operations.ActiveReservation().Activity)],Operations.ActiveParticipants(),VisiblePeople,Operations.ActiveDelayMinutes());
    }
    return FString::Printf(TEXT("Raccorde / %.0f m / trajet %.0f min / %d figurants"),RouteMetres,FMath::CeilToDouble(RouteMetres/72.),VisiblePeople);
}

void ACampusBuilding::UpdateFigures(double Seconds,float DeltaSeconds,bool Paused,ACampusPaths* Paths)
{
    if(FigureBodies->GetInstanceCount()==0)
    {
        for(int I=0;I<8;++I)
        {
            FigureBodies->AddInstance(FTransform::Identity); FigureHeads->AddInstance(FTransform::Identity);
            for(int L=0;L<4;++L) { FigureLimbs->AddInstance(FTransform::Identity); }
        }
    }
    if(!Paused) { AnimationTime+=DeltaSeconds; }
    const double Minute=Seconds/60.; const int64 Day=static_cast<int64>(Minute/1440);
    const FCampusReservation* Candidate=nullptr;
    double Start=0,Arrival=0; int Count=8;
    if(Operations.IsActive())
    {
        if(Operations.ActiveCanRun())
        { Candidate=&Operations.ActiveReservation(); Start=Operations.ActiveStartMinute(); Arrival=Operations.ActiveArrivalMinute(); Count=FMath::Min(8,Operations.ActiveParticipants()); }
        else { VisualId=0; }
    }
    else if(Paths && Paths->IsConnected())
    {
        const double Travel=FMath::CeilToDouble(Paths->DistanceMetres()/72.);
        for(int H=static_cast<int>(Minute/60);H<=static_cast<int>((Minute+Travel)/60)+1;++H)
        {
            const auto* R=Schedule.FindAt((H/24)%7,H%24);
            if(!R || R->StartHour!=H%24 || Operations.StartedThisWeek(R->Id)) { continue; }
            if(CampusNeedsStaff(R->Activity) && R->StaffId==0) { continue; }
            if(R->Activity==ECampusActivity::ClubRental && (!R->Contract || Operations.ContractState!=1 || Operations.ContractWeek!=H/168)) { continue; }
            const double A=FMath::Max(H*60.,FMath::Max(Paths->ReadyMinute,static_cast<double>(R->PlannedAtMinute))+Travel);
            if(Minute>=A-Travel && Minute<H*60.)
            {
                Count=R->Activity==ECampusActivity::PublicLesson ? FMath::Min(8,FCampusOperations::PublicParticipants(R->StartHour,R->Price)) : 8;
                if(Count>0) { Candidate=R; Start=H*60.; Arrival=A; }
                break;
            }
        }
    }
    if(!Operations.IsActive() && (!Paths || !Paths->IsConnected()) && Minute<VisualStart) { VisualId=0; }
    if(Candidate && (Candidate->Id!=VisualId || VisualDay!=Day || (Operations.IsActive() && !bVisualStarted)))
    {
        bVisualStarted=Operations.IsActive();
        VisualId=Candidate->Id; VisualDay=Day; VisualReservation=*Candidate;
        VisualStart=Start; VisualArrival=Arrival; VisualEnd=Start+Candidate->Duration*60.;
        VisualTravel=Paths ? FMath::CeilToDouble(Paths->DistanceMetres()/72.) : 0;
        if(Operations.GroupAlreadyHere(Candidate->Group,Day)) { VisualTravel=0; }
        VisualPeopleCount=Count;
        VisualRoute=Paths ? Paths->RoutePoints() : TArray<FVector>();
        FigureBodies->SetMaterial(0,FigureMaterials[static_cast<int>(Candidate->Activity)]);
    }
    if(Candidate && Operations.IsActive()) { VisualArrival=Arrival; }
    const bool Departing=Minute>=VisualEnd && VisualReservation.Group==ECampusGroup::None;
    const bool Show=VisualId && VisualDay==Day && (!Departing || Minute<VisualEnd+VisualTravel);
    VisiblePeople=Show ? VisualPeopleCount : 0;
    FigureBodies->SetVisibility(Show); FigureHeads->SetVisibility(Show); FigureLimbs->SetVisibility(Show);
    if(!Show) { return; }
    auto RoutePoint=[this](double Fraction)
    {
        if(VisualRoute.IsEmpty()) { return GetActorLocation()-FVector(0,0,400); }
        double Total=0; for(int I=1;I<VisualRoute.Num();++I) { Total+=FVector::Distance(VisualRoute[I-1],VisualRoute[I]); }
        double Remaining=FMath::Clamp(Fraction,0.,1.)*Total;
        for(int I=1;I<VisualRoute.Num();++I)
        { const double L=FVector::Distance(VisualRoute[I-1],VisualRoute[I]); if(Remaining<=L) { return FMath::Lerp(VisualRoute[I-1],VisualRoute[I],L>0?Remaining/L:0.); } Remaining-=L; }
        return VisualRoute.Last();
    };
    const bool Walking=Minute<VisualArrival || Departing;
    for(int I=0;I<8;++I)
    {
        FVector Base; double Yaw=GetActorRotation().Yaw;
        if(Walking)
        {
            double Fraction=VisualTravel>0 ? (Minute-(VisualArrival-VisualTravel))/VisualTravel : 1.;
            if(Departing) { Fraction=1.-(Minute-VisualEnd)/FMath::Max(1.,VisualTravel); }
            Fraction-=I*.006;
            Base=RoutePoint(Fraction); const FVector Next=RoutePoint(Fraction+.002);
            Yaw=(Next-Base).Rotation().Yaw+(Departing?180:0);
        }
        else
        {
            const double Phase=AnimationTime*.7+I*.8;
            FVector Local((I%4-1.5)*300,(I/4-.5)*450,-400);
            if(Minute<VisualEnd)
            {
                if(VisualReservation.Activity==ECampusActivity::Training)
                { Local.X=FMath::Cos(Phase)*1000; Local.Y=FMath::Sin(Phase)*500; Yaw+=Phase*180/PI+90; }
                else if(VisualReservation.Activity==ECampusActivity::PublicLesson)
                { Local.Z+=FMath::Max(0.,FMath::Sin(AnimationTime*2))*12; }
                else if(VisualReservation.Activity!=ECampusActivity::University)
                { Local.X+=FMath::Sin(Phase)*150; Local.Y+=FMath::Cos(Phase)*120; Yaw+=Phase*30; }
            }
            Base=GetActorTransform().TransformPosition(Local);
        }
        const double Bob=Walking ? FMath::Sin(AnimationTime*10+I)*3 : FMath::Sin(AnimationTime*2+I)*2;
        const FVector Scale=I<VisiblePeople?FVector(1):FVector::ZeroVector;
        FigureBodies->UpdateInstanceTransform(I,FTransform(FRotator(0,Yaw,0),Base+FVector(0,0,90+Bob),Scale*FVector(.4,.3,.8)),true,false);
        FigureHeads->UpdateInstanceTransform(I,FTransform(FRotator::ZeroRotator,Base+FVector(0,0,150+Bob),Scale*.36),true,false);
        for(int L=0;L<4;++L)
        {
            const double Swing=FMath::Sin(AnimationTime*(Walking?10:3)+I+(L%2)*PI)*(Walking?25:8);
            const FVector Offset=FRotator(0,Yaw,0).RotateVector(FVector(0,(L%2==0?-1:1)*(L<2?12:25),L<2?32:98));
            FigureLimbs->UpdateInstanceTransform(I*4+L,FTransform(FRotator(Swing,Yaw,0),Base+Offset,FVector(.12,.12,.6)*Scale),true,false);
        }
    }
    FigureBodies->MarkRenderStateDirty(); FigureHeads->MarkRenderStateDirty(); FigureLimbs->MarkRenderStateDirty();
}
