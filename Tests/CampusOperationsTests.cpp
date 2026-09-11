#include "../Source/TycoonCampus/CampusOperations.h"
#include "../Source/TycoonCampus/CampusTimeModel.h"
#include <iostream>
#include <stdexcept>
static void Check(bool Value, const char* Message) { if (!Value) { throw std::runtime_error(Message); } }
static FCampusWeeklySchedule Reference()
{
    FCampusWeeklySchedule S;
    S.HireStaff();
    const auto U = ECampusActivity::University, T = ECampusActivity::Training, P = ECampusActivity::PublicLesson, F = ECampusActivity::FreeAccess;
    const auto OK = ECampusBookingResult::Success;
    Check(S.Add(0, 8, 2, U, -1, 1) == OK && S.Add(0, 14, 2, T, -1, 1) == OK && S.Add(0, 18, 2, P, -1, 1) == OK, "reference monday");
    Check(S.Add(1, 16, 2, F) == OK && S.Add(2, 8, 2, U, -1, 1) == OK && S.Add(2, 18, 2, P, -1, 1) == OK, "reference midweek");
    Check(S.Add(3, 14, 2, T, -1, 1) == OK && S.Add(3, 16, 2, F) == OK && S.Add(4, 8, 2, U, -1, 1) == OK, "reference endweek");
    Check(S.Add(4, 20, 2, ECampusActivity::ClubRental, -1, 0, 12, true) == OK, "reference club");
    return S;
}
int main()
{
    try
    {
        const auto OK = ECampusBookingResult::Success, U = ECampusBookingResult::StaffUnavailable;
        const auto P = ECampusActivity::PublicLesson;
        FCampusWeeklySchedule S;
        Check(S.Add(0, 18, 2, P, -1, 1) == ECampusBookingResult::InvalidStaff, "unhired staff assigned");
        S.HireStaff(); S.HireStaff();
        Check(S.Add(0, 18, 2, P, -1, 1) == OK, "valid staffed public");
        Check(S.Add(1, 19, 2, P, -1, 1) == U && S.Add(5, 18, 2, P, -1, 1) == U, "availability ignored");
        Check(S.Add(1, 18, 2, P, -1, 1, 31) == ECampusBookingResult::InvalidPrice, "invalid price");
        Check(S.Add(1, 18, 2, ECampusActivity::FreeAccess, -1, 1) == ECampusBookingResult::InvalidStaff, "irrelevant staff");
        Check(S.Update(1, 5, 18, 2, P, -1, 1) == U && S.FindById(1)->Day == 0, "invalid edit mutated original");
        Check(S.Add(1, 20, 2, ECampusActivity::ClubRental, -1, 0, 12, true) == ECampusBookingResult::InvalidContract, "contract wrong day");
        Check(S.GetStaffHours() == 2, "staff load");
        FCampusOperations O;
        O.AdvanceTo(18, S); Check(O.Cash == 100000 && O.IsActive(), "revenue paid before completion");
        O.AdvanceTo(20, S); Check(O.Cash == 100220 && O.Week.Revenue == 240 && O.Week.Expense == 20, "public net income");
        const int Count = O.EntryCount(); O.AdvanceTo(20, S); O.AdvanceTo(19, S);
        Check(O.Cash == 100220 && O.EntryCount() == Count, "duplicate tick pays twice");
        Check(FCampusOperations::PublicParticipants(18, 0) == 20 && FCampusOperations::PublicParticipants(18, 12) == 20 && FCampusOperations::PublicParticipants(18, 30) == 2 && FCampusOperations::PublicParticipants(9, 12) == 8, "demand/capacity");

        FCampusWeeklySchedule Missing;
        Check(Missing.Add(0, 8, 2, ECampusActivity::University) == OK, "unstaffed booking rejected");
        FCampusOperations M;
        M.AdvanceTo(8, Missing); Missing.HireStaff();
        Check(Missing.Update(1, 0, 8, 2, ECampusActivity::University, -1, 1) == OK, "mid-session staffing");
        M.AdvanceTo(10, Missing);
        Check(M.Week.Missed == 1 && M.Week.UniversityHours == 0 && M.Cash == 100000, "staffing rescued already started occurrence");

        FCampusWeeklySchedule Snap; Snap.HireStaff();
        Check(Snap.Add(0, 18, 2, P, -1, 1, 12) == OK, "snapshot setup");
        FCampusOperations A; A.AdvanceTo(18, Snap);
        Check(Snap.Update(1, 1, 18, 2, P, -1, 1, 30) == OK, "snapshot move");
        A.AdvanceTo(20, Snap); Check(A.Week.Revenue == 240, "active price changed retroactively");
        A.AdvanceTo(44, Snap); Check(A.Week.Revenue == 240, "moved same occurrence ran twice");
        FCampusWeeklySchedule Deleted; Deleted.HireStaff();
        Check(Deleted.Add(0, 18, 2, P, -1, 1) == OK, "delete snapshot setup");
        FCampusOperations D; D.AdvanceTo(18, Deleted); Deleted.Remove(1);
        Check(Deleted.Add(0, 19, 1, ECampusActivity::FreeAccess) == OK, "shadow overlap setup");
        D.AdvanceTo(20, Deleted);
        Check(D.Week.Revenue == 240 && D.Week.Completed == 1 && D.Week.Missed == 1, "deleted snapshot or occupied occurrence broken");
        FCampusWeeklySchedule Zero; Zero.HireStaff();
        Check(Zero.Add(0, 9, 2, P, -1, 1, 30) == OK, "zero demand setup");
        FCampusOperations Z; Z.AdvanceTo(11, Zero);
        Check(Z.Week.Missed == 1 && Z.Cash == 100000 && Z.RecentEntry(0).Kind == ECampusEntry::NoDemand, "zero demand charged");

        FCampusOperations C; FCampusWeeklySchedule Empty;
        Check(C.AcceptContract() && !C.AcceptContract(), "contract double accept");
        Check(Empty.Add(4, 20, 2, ECampusActivity::ClubRental, -1, 0, 12, true) == OK, "contract delete setup");
        Empty.Remove(1); Check(C.ContractState == 1 && !Empty.HasContractBooking(), "deleting booking erased obligation");
        C.AdvanceTo(118, Empty); Check(C.ContractState == 3 && C.Cash == 99600, "unprogrammed contract not missed");
        FCampusOperations Full; auto R = Reference();
        Check(Full.AcceptContract(), "reference contract");
        Full.AdvanceTo(168, R);
        Check(Full.PreviousWeek.Revenue == 1980 && Full.PreviousWeek.Expense == 1500 && Full.PreviousWeek.Profit() == 480 && Full.Cash == 100480, "reference weekly finances");
        Check(Full.PreviousWeek.UniversityHours == 6 && Full.PreviousWeek.TrainingHours == 4 && Full.PreviousWeek.Completed == 10 && Full.PreviousWeek.Missed == 0, "reference real attendance");
        Full.AdvanceTo(336, R); Check(Full.PreviousWeek.Revenue == 1680 && Full.PreviousWeek.Missed == 1, "one-off contract paid again");

        FCampusOperations Slow, Fast; Slow.AcceptContract(); Fast.AcceptContract();
        for (int H = 8; H <= 168; ++H) { Slow.AdvanceTo(H, R); }
        Fast.AdvanceTo(168, R);
        Check(Slow.Cash == Fast.Cash && Slow.EntryCount() == Fast.EntryCount(), "frame batch dependence");
        for (int I = 0; I < Slow.EntryCount(); ++I)
        { Check(Slow.RecentEntry(I).Hour == Fast.RecentEntry(I).Hour && Slow.RecentEntry(I).Revenue == Fast.RecentEntry(I).Revenue && Slow.RecentEntry(I).Expense == Fast.RecentEntry(I).Expense, "journal mismatch"); }
        FCampusTimeModel Clock; FCampusOperations Timed; Timed.AcceptContract();
        Clock.Advance(100, [](const auto&){}); Check(Clock.GetTotalMinutes() == 480, "pause drift");
        Clock.SetSpeed(4); Timed.AdvanceTo(8, R);
        Clock.Advance(500, [&](const auto& B) { Timed.AdvanceTo(B.AbsoluteHour, R); });
        Check(Timed.Cash == 100480, "accelerated clock integration");
        FCampusWeeklySchedule NoIncome; NoIncome.HireStaff(); FCampusOperations Debt;
        Debt.AdvanceTo(168 * 1001, NoIncome); Check(Debt.Cash == -100 && Debt.EntryCount() == 128, "deficit or ledger ring");
        std::cout << "PASS: staffing, availability, salary, public demand, cash timing, snapshot, contracts, reference +480, repeat week, x4, batched time, deficit, journal\n";
        return 0;
    }
    catch (const std::exception& E) { std::cerr << "FAIL: " << E.what() << '\n'; return 1; }
}
