#pragma once

#include <cmath>
#include <cstdint>

// Engine-independent clock rules, also exercised by the standalone regression tests.
class FCampusTimeModel
{
    friend struct FCampusSaveCodec;
public:
    static constexpr double SimulatedSecondsPerRealSecond = 288.0;

    struct FBoundary
    {
        std::int64_t AbsoluteHour;
        bool StartsDay;
        bool StartsWeek;
        bool StartsSemester;
    };

    bool IsPaused() const { return bPaused; }
    double GetTotalSeconds() const { return Seconds; }
    int GetSpeed() const { return Speed; }
    std::int64_t GetTotalMinutes() const { return static_cast<std::int64_t>(Seconds / 60.0); }
    void TogglePause() { bPaused = !bPaused; }
    void Pause() { bPaused = true; }

    bool SetSpeed(int NewSpeed)
    {
        if (NewSpeed != 1 && NewSpeed != 2 && NewSpeed != 4) { return false; }
        Speed = NewSpeed;
        bPaused = false;
        return true;
    }

    template <typename FCallback>
    void Advance(double RealSeconds, FCallback&& OnBoundary)
    {
        if (bPaused || bAdvancing || !std::isfinite(RealSeconds) || RealSeconds <= 0.0) { return; }
        // Compensated summation avoids losing a boundary to accumulated rounding.
        const double Increment = RealSeconds * SimulatedSecondsPerRealSecond * Speed - RoundingCorrection;
        const double Destination = Seconds + Increment;
        const double NextCorrection = (Destination - Seconds) - Increment;
        // Bound conversions to int64, including malformed or extreme delta input.
        if (!std::isfinite(Destination) || Destination >= 9.0e18) { return; }
        const auto FirstHour = static_cast<std::int64_t>(Seconds / 3600.0) + 1;
        const auto LastHour = static_cast<std::int64_t>(Destination / 3600.0);
        bAdvancing = true;
        for (auto Hour = FirstHour; Hour <= LastHour; ++Hour)
        {
            // Subscribers observe the boundary's time, not the final frame time.
            Seconds = static_cast<double>(Hour) * 3600.0;
            OnBoundary(FBoundary{Hour, Hour % 24 == 0, Hour % 168 == 0, Hour % 672 == 0});
        }
        Seconds = Destination;
        RoundingCorrection = NextCorrection;
        bAdvancing = false;
    }

private:
    double Seconds = 8.0 * 3600.0;
    double RoundingCorrection = 0.0;
    bool bPaused = true;
    bool bAdvancing = false;
    int Speed = 1;
};
