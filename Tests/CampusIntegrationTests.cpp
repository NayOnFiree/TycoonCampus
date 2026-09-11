#include "../Source/TycoonCampus/CampusSaveCodec.h"
#include "../Source/TycoonCampus/CampusPathGrid.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>

static void Check(bool Value, const char* Message)
{ if (!Value) { throw std::runtime_error(Message); } }

static std::vector<unsigned char> Encode(FCampusSaveState& State)
{
    std::vector<unsigned char> Bytes(FCampusSaveCodec::MaxBytes);
    std::size_t Size = 0;
    Check(FCampusSaveCodec::Encode(State, Bytes.data(), Bytes.size(), Size), "invalid integrated state");
    Bytes.resize(Size);
    return Bytes;
}

static void Advance(FCampusSaveState& State, double RealSeconds)
{
    State.Clock.Advance(RealSeconds, [](auto) {});
    if (!State.Clock.IsPaused())
    { State.Operations.AdvanceTo(State.Clock.GetTotalMinutes() / 60, State.Schedule); }
}

int main()
{
    try
    {
        auto State = std::make_unique<FCampusSaveState>();
        auto Grid = std::make_unique<FCampusPathGrid>();
        Check(State->Operations.PurchaseGym(480), "construction failed");
        State->Built = true;
        const auto Footprint = FCampusFootprint::AtCursor(700, 0, false);
        State->BuildingX = Footprint.CenterX(); State->BuildingY = Footprint.CenterY();
        Grid->Cells[101] = 1;
        Check(Grid->MakeSegment(1, 1, 5, 1, false), "path preview failed");
        const int Price = Grid->Quote(false);
        Check(Price == 80 && Grid->Apply(false, State->Operations.Cash), "path purchase failed");
        State->Operations.RecordPathInvestment(Price, 480);
        Check(Grid->FindRoute(101, 105), "purchased path disconnected");
        State->Operations.SetAccess(true, (Grid->RouteCount - 1) * 2., 480);
        for (int I = 0; I < 10000; ++I) { State->Paths[I] = Grid->Cells[I]; }
        State->PathReadyMinute = 480;
        State->Schedule.HireStaff();
        Check(State->Schedule.Add(0, 18, 2, ECampusActivity::PublicLesson, -1, 1) == ECampusBookingResult::Success, "booking failed");
        State->Clock.SetSpeed(1);
        Advance(*State, 10.5 * 3600 / 288.);
        Check(State->Operations.IsActive(), "session did not start");
        Check(State->Operations.Cash == 39920, "income posted before completion");
        auto Bytes = Encode(*State);
        auto Resumed = std::make_unique<FCampusSaveState>();
        Check(FCampusSaveCodec::Decode(Bytes.data(), Bytes.size(), *Resumed), "resume failed");
        Resumed->Clock.Pause();
        const auto Paused = Encode(*Resumed);
        Advance(*Resumed, 100);
        Check(Encode(*Resumed) == Paused, "pause changes simulation");
        Resumed->Clock.SetSpeed(1);
        Advance(*State, 1.5 * 3600 / 288.);
        Advance(*Resumed, 1.5 * 3600 / 288.);
        Check(State->Operations.Cash == 40140, "construction/path/session net is wrong");
        Check(Encode(*State) == Encode(*Resumed), "save during activity changes outcome");

        // Same 16-week simulation with hourly frames versus one large catch-up.
        auto Stepped = std::make_unique<FCampusSaveState>(*State);
        constexpr int Hours = 16 * 168;
        Advance(*State, Hours * 3600. / 288.);
        for (int H = 0; H < Hours; ++H) { Advance(*Stepped, 3600. / 288.); }
        Check(Encode(*State) == Encode(*Stepped), "long-run result depends on frame partition");
        Check(State->Operations.EntryCount() == 128, "ledger ring did not wrap");
        Check(State->Operations.HasPreviousWeek, "weekly closing missing");
        std::cout << "Integration: construction, paths, booking, pause, resume and 16-week determinism passed\n";
        return 0;
    }
    catch (const std::exception& Error) { std::cerr << Error.what() << '\n'; return 1; }
}
