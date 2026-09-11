#include "../Source/TycoonCampus/CampusTimeModel.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

static void Check(bool Condition, const char* Message)
{
    if (!Condition) { throw std::runtime_error(Message); }
}

int main()
{
    try
    {
        auto Ignore = [](const FCampusTimeModel::FBoundary&) {};
        for (int Speed : {1, 2, 4})
        {
            FCampusTimeModel Time;
            Time.SetSpeed(Speed);
            std::vector<std::int64_t> Hours;
            int Days = 0;
            Time.Advance(300.0 / Speed, [&](const auto& B)
            {
                Check(Time.GetTotalMinutes() == B.AbsoluteHour * 60, "callback sees wrong time");
                Hours.push_back(B.AbsoluteHour);
                Days += B.StartsDay;
            });
            Check(Time.GetTotalMinutes() == 1920, "day duration differs by speed");
            Check(Hours.size() == 24 && Hours.front() == 9 && Hours.back() == 32, "hours skipped in a large frame");
            Check(Days == 1, "midnight missing or duplicated");
        }
        FCampusTimeModel Paused;
        int Count = 0;
        auto CountBoundary = [&](const auto&) { ++Count; };
        Paused.Advance(300, CountBoundary);
        Check(Count == 0 && Paused.GetTotalMinutes() == 480, "initial pause advances time");
        Paused.SetSpeed(4);
        Paused.TogglePause();
        Paused.Advance(300, CountBoundary);
        Check(Count == 0 && Paused.GetSpeed() == 4, "pause loses speed");
        Paused.TogglePause();
        Paused.Advance(3.125, CountBoundary);
        Check(Count == 1 && Paused.GetTotalMinutes() == 540, "exact hour boundary not emitted");
        Paused.Advance(0, CountBoundary);
        Paused.Advance(-1, CountBoundary);
        Paused.Advance(std::numeric_limits<double>::quiet_NaN(), CountBoundary);
        Check(Count == 1 && !Paused.SetSpeed(3) && Paused.GetSpeed() == 4, "invalid input changes state");

        FCampusTimeModel Semester;
        Semester.SetSpeed(1);
        int Hours = 0, Days = 0, Weeks = 0, Semesters = 0;
        Semester.Advance(8300, [&](const auto& B)
        {
            ++Hours; Days += B.StartsDay; Weeks += B.StartsWeek; Semesters += B.StartsSemester;
        });
        Check(Hours == 664 && Days == 28 && Weeks == 4 && Semesters == 1, "semester transition counts wrong");
        Check(Semester.GetTotalMinutes() == 28 * 1440, "semester date wrong");
        Semester.Advance(1, [&](const auto& B) { Semesters += B.StartsSemester; });
        Check(Semesters == 1, "semester emitted twice");

        FCampusTimeModel Large, Small;
        Large.SetSpeed(4); Small.SetSpeed(4);
        std::vector<std::int64_t> A, B;
        Large.Advance(100, [&](const auto& E) { A.push_back(E.AbsoluteHour); });
        for (int I = 0; I < 1000; ++I) { Small.Advance(.1, [&](const auto& E) { B.push_back(E.AbsoluteHour); }); }
        Check(A == B && Large.GetTotalMinutes() == Small.GetTotalMinutes(), "frame-size changes timeline");

        FCampusTimeModel Switching;
        Switching.SetSpeed(1); Switching.Advance(10, Ignore);
        Switching.SetSpeed(2); Switching.Advance(10, Ignore);
        Switching.SetSpeed(4); Switching.Advance(10, Ignore);
        Check(Switching.GetTotalMinutes() == 816, "speed switch loses elapsed time");
        std::cout << "PASS: base speeds, pause/resume, skipped hours, midnight, weeks, semester, invalid inputs, frame independence, speed switches\n";
        return 0;
    }
    catch (const std::exception& Error)
    {
        std::cerr << "FAIL: " << Error.what() << '\n';
        return 1;
    }
}
