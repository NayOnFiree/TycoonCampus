#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampusTimeModel.h"
#include "CampusClock.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FCampusTimeBoundaryEvent, int64);

UCLASS()
class TYCOONCAMPUS_API ACampusClock : public AActor
{
    GENERATED_BODY()
    friend struct FCampusSaveService;
public:
    ACampusClock();
    virtual void Tick(float DeltaSeconds) override;
    void TogglePause() { Time.TogglePause(); }
    bool IsSimulationPaused() const { return Time.IsPaused(); }
    void SetSimulationSpeed(int32 NewSpeed);
    int32 GetSimulationSpeed() const { return Time.GetSpeed(); }
    int64 GetTotalMinutes() const { return Time.GetTotalMinutes(); }
    double GetTotalSeconds() const { return Time.GetTotalSeconds(); }

    // Absolute zero-based indices. Order at midnight: hour, day, week, semester.
    FCampusTimeBoundaryEvent OnHourStarted;
    FCampusTimeBoundaryEvent OnDayStarted;
    FCampusTimeBoundaryEvent OnWeekStarted;
    FCampusTimeBoundaryEvent OnSemesterStarted;
private:
    FCampusTimeModel Time;
};
