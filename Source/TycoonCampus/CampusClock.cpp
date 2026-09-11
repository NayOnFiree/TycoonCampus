#include "CampusClock.h"

ACampusClock::ACampusClock()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACampusClock::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Time.Advance(static_cast<double>(DeltaSeconds), [this](const FCampusTimeModel::FBoundary& Boundary)
    {
        const int64 Hour = Boundary.AbsoluteHour;
        OnHourStarted.Broadcast(Hour);
        if (Boundary.StartsDay) { OnDayStarted.Broadcast(Hour / 24); }
        if (Boundary.StartsWeek) { OnWeekStarted.Broadcast(Hour / 168); }
        if (Boundary.StartsSemester) { OnSemesterStarted.Broadcast(Hour / 672); }
        UE_LOG(LogTemp, Verbose, TEXT("CampusClock: hour %lld dispatched"), Hour);
    });
}

void ACampusClock::SetSimulationSpeed(int32 NewSpeed)
{
    if (Time.SetSpeed(NewSpeed))
    {
        UE_LOG(LogTemp, Display, TEXT("CampusClock: running x%d (base day = 5 real minutes)"), Time.GetSpeed());
    }
}
