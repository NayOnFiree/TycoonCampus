#include "../Source/TycoonCampus/CampusWeeklySchedule.h"
#include <iostream>
#include <stdexcept>

static void Check(bool Value, const char* Message) { if (!Value) { throw std::runtime_error(Message); } }
int main()
{
    try
    {
        FCampusWeeklySchedule Schedule;
        const auto U = ECampusActivity::University;
        const auto OK = ECampusBookingResult::Success;
        const auto Busy = ECampusBookingResult::Occupied;
        FCampusWeeklySchedule Groups;
        Check(Groups.GetNeeds(ECampusGroup::Students).Members == 24 && Groups.GetNeeds(ECampusGroup::Students).MissingSessions() == 3, "student needs incorrect");
        Check(Groups.GetNeeds(ECampusGroup::Team).Members == 16 && Groups.GetNeeds(ECampusGroup::Team).MissingSessions() == 2, "team needs incorrect");
        Check(Groups.Add(0, 8, 2, U, 2) == ECampusBookingResult::InvalidGroup && Groups.GetReservationCount() == 0, "wrong group accepted");
        Check(Groups.Add(0, 8, 2, U, 99) == ECampusBookingResult::InvalidGroup, "unknown group accepted");
        Check(Groups.Add(0, 8, 2, U) == OK && Groups.Add(2, 8, 2, U) == OK && Groups.Add(4, 8, 2, U) == OK, "student week failed");
        Check(Groups.GetNeeds(ECampusGroup::Students).PlannedHours == 6 && Groups.GetNeeds(ECampusGroup::Students).MissingSessions() == 0, "student coverage incorrect");
        Check(Groups.Add(0, 14, 2, ECampusActivity::Training) == OK && Groups.Add(3, 14, 2, ECampusActivity::Training) == OK, "team week failed");
        Check(Groups.GetNeeds(ECampusGroup::Team).PlannedHours == 4 && Groups.GetNeeds(ECampusGroup::Team).MissingSessions() == 0, "team coverage incorrect");
        Check(Groups.Add(1, 14, 2, ECampusActivity::FreeAccess, 1) == OK, "student free access rejected");
        Check(Groups.GetNeeds(ECampusGroup::Students).PlannedHours == 6, "free access counted as university");
        Check(Groups.Update(1, 0, 8, 2, U, 2) == ECampusBookingResult::InvalidGroup && Groups.FindById(1)->Group == ECampusGroup::Students, "invalid group edit mutated original");
        Check(Groups.Update(1, 0, 8, 4, U, 1) == OK && Groups.GetNeeds(ECampusGroup::Students).PlannedHours == 8 && Groups.GetNeeds(ECampusGroup::Students).MissingSessions() == 1, "long session falsely covers session target");
        Check(Groups.Update(1, 1, 8, 2, U, 1) == OK && Groups.GetNeeds(ECampusGroup::Students).MissingSessions() == 0, "move coverage incorrect");
        Check(Groups.Update(1, 1, 8, 2, ECampusActivity::FreeAccess, 1) == OK && Groups.GetNeeds(ECampusGroup::Students).MissingSessions() == 1, "activity conversion retained credit");
        Check(Groups.Remove(2) && Groups.GetNeeds(ECampusGroup::Students).MissingSessions() == 2, "deletion retained coverage");
        Check(Groups.GetNeeds(ECampusGroup::None).RequiredSessions == 0, "public given student objectives");
        std::cout << "PASS: group compatibility, objectives, duration alerts, editing, deletion, free access exclusion\n";
        Check(Schedule.Add(0, 10, 2, U) == OK, "creation failed");
        Check(!Schedule.FindAt(0, 9) && Schedule.FindAt(0, 10) && Schedule.FindAt(0, 11) && !Schedule.FindAt(0, 12), "half-open interval broken");
        Check(Schedule.Add(0, 8, 2, U) == OK && Schedule.Add(0, 12, 1, U) == OK, "adjacent reservations rejected");
        Check(Schedule.Add(0, 10, 2, U) == Busy, "duplicate allowed");
        Check(Schedule.Add(0, 11, 1, U) == Busy, "contained interval allowed");
        Check(Schedule.Add(0, 9, 4, U) == Busy, "enclosing interval allowed");
        Check(Schedule.Add(1, 10, 2, U) == OK, "separate days conflict");
        Check(Schedule.Add(6, 21, 1, ECampusActivity::ClubRental) == OK, "last slot rejected");
        Check(Schedule.Add(2, 21, 2, U) == ECampusBookingResult::OutsideOpeningHours, "closing time ignored");
        Check(Schedule.Add(7, 10, 1, U) == ECampusBookingResult::InvalidDay, "invalid day accepted");
        Check(Schedule.Add(-1, 10, 1, U) == ECampusBookingResult::InvalidDay, "negative day accepted");
        Check(Schedule.Add(2, 7, 1, U) == ECampusBookingResult::InvalidStart, "early start accepted");
        Check(Schedule.Add(2, 22, 1, U) == ECampusBookingResult::InvalidStart, "late start accepted");
        Check(Schedule.Add(2, 10, 0, U) == ECampusBookingResult::InvalidDuration, "zero duration accepted");
        Check(Schedule.Add(2, 10, 2147483647, U) == ECampusBookingResult::InvalidDuration, "overflow duration accepted");
        Check(Schedule.Add(2, 10, 1, static_cast<ECampusActivity>(99)) == ECampusBookingResult::InvalidActivity, "invalid activity accepted");
        Check(Schedule.GetReservationCount() == 5, "failed creation mutated reservations");
        Check(Schedule.Add(3, 8, 14, ECampusActivity::FreeAccess) == OK, "full opening day rejected");
        Check(Schedule.FindAt(3, 8)->Id == 6, "ids consumed on errors");
        Check(Schedule.FindAt(3, 21)->Activity == ECampusActivity::FreeAccess, "activity not retained across duration");
        FCampusWeeklySchedule FullWeek;
        Check(FullWeek.GetReservationCount() == 0 && !FullWeek.FindAt(0, 8), "empty week broken");
        for (int Day = 0; Day < 7; ++Day)
        {
            for (int Hour = 8; Hour < 22; ++Hour)
            {
                Check(FullWeek.Add(Day, Hour, 1, U) == OK, "full week slot rejected");
            }
        }
        Check(FullWeek.GetReservationCount() == 98, "full week count incorrect");
        Check(FullWeek.Add(6, 21, 1, U) == Busy && FullWeek.GetReservationCount() == 98, "full week overflow");
        Check(FullWeek.FindAt(6, 21)->Id == 98, "last reservation lost");
        Check(FullWeek.GetBookedHours() == 98, "full occupancy incorrect");
        Check(FullWeek.Update(98, 6, 21, 1, ECampusActivity::Training) == OK, "update in full week rejected");
        Check(FullWeek.Update(98, 6, 20, 1, U) == Busy, "update collision accepted");
        Check(FullWeek.FindById(98)->StartHour == 21 && FullWeek.FindById(98)->Activity == ECampusActivity::Training, "failed update changed original");
        Check(FullWeek.Remove(50) && !FullWeek.FindById(50) && FullWeek.GetReservationCount() == 97, "middle deletion broken");
        Check(FullWeek.FindById(98) && FullWeek.GetBookedHours() == 97, "deletion damaged another booking");
        Check(FullWeek.Add(3, 15, 1, U) == OK && FullWeek.FindAt(3, 15)->Id == 99, "deleted slot not reusable or id recycled");

        FCampusWeeklySchedule Editing;
        Check(Editing.Add(0, 10, 2, U) == OK && Editing.Add(0, 12, 2, U) == OK, "edit setup failed");
        Check(Editing.Update(1, 0, 9, 3, ECampusActivity::Training) == OK, "self-overlap rejected");
        Check(Editing.GetBookedHours() == 5 && Editing.GetReservationCount() == 2, "edit totals wrong");
        Check(Editing.Update(1, 0, 11, 2, U) == Busy && Editing.FindById(1)->StartHour == 9, "conflicting edit not atomic");
        Check(Editing.FindConflict(0, 11, 2, 1)->Id == 2, "conflict identifies wrong booking");
        Check(Editing.Update(1, 2, 20, 3, U) == ECampusBookingResult::OutsideOpeningHours, "invalid edit accepted");
        Check(Editing.Update(1, -1, 10, 1, U) == ECampusBookingResult::InvalidDay, "invalid edit day accepted");
        Check(Editing.Update(1, 2, 10, 2147483647, U) == ECampusBookingResult::InvalidDuration, "invalid edit duration accepted");
        Check(Editing.FindById(1)->Day == 0 && Editing.GetBookedHours() == 5, "invalid edit mutated booking");
        Check(Editing.Update(1, 6, 21, 1, ECampusActivity::ClubRental) == OK, "move day failed");
        Check(!Editing.FindAt(0, 9) && Editing.FindAt(6, 21)->Id == 1, "move did not free original hours");
        Check(Editing.Update(999, 0, 8, 1, U) == ECampusBookingResult::NotFound && !Editing.Remove(999), "missing id accepted");
        Check(Editing.Remove(1) && Editing.FindById(2) && Editing.Remove(2), "first/last deletion failed");
        Check(Editing.GetBookedHours() == 0 && Editing.GetReservationCount() == 0 && !Editing.FindAt(0, 12), "deletion left occupied slots");
        const auto Public = ECampusActivity::PublicLesson;
        const auto Club = ECampusActivity::ClubRental;
        FCampusWeeklySchedule Preparation;
        Check(Preparation.Add(0, 8, 2, Public) == ECampusBookingResult::OutsideOpeningHours, "preparation before opening accepted");
        Check(Preparation.Add(0, 9, 2, Club) == OK, "preparation at opening rejected");
        Check(Preparation.FindAt(0, 8)->Id == Preparation.FindAt(0, 9)->Id, "preparation not linked to booking");
        Check(Preparation.Add(0, 8, 1, U) == Busy, "existing preparation overwritten");
        Check(Preparation.Add(0, 11, 1, Public) == Busy, "new preparation overlaps existing activity");
        Check(Preparation.Add(0, 12, 2, Public) == OK, "adjacent preparation rejected");
        Check(Preparation.GetBookedHours() == 4 && Preparation.GetPreparationHours() == 2, "preparation counted as activity or twice");
        Check(Preparation.Update(2, 0, 11, 2, Public) == Busy && Preparation.FindById(2)->StartHour == 12, "failed prep update mutated original");
        Check(Preparation.Update(2, 0, 12, 2, U) == OK && !Preparation.FindAt(0, 11), "activity change left preparation behind");
        Check(Preparation.Add(0, 11, 1, U) == OK, "released preparation not reusable");
        Check(Preparation.Update(2, 0, 12, 2, Public) == Busy && Preparation.FindById(2)->Activity == U, "new prep ignored on activity change");
        Check(Preparation.Update(1, 1, 20, 2, Public) == OK && !Preparation.FindAt(0, 8) && Preparation.FindAt(1, 19), "move did not move preparation");
        Check(Preparation.Remove(1) && !Preparation.FindAt(1, 19) && !Preparation.FindAt(1, 20), "delete left preparation behind");
        FCampusWeeklySchedule Reference;
        Check(Reference.Add(0, 8, 2, U) == OK && Reference.Add(0, 14, 2, ECampusActivity::Training) == OK && Reference.Add(0, 18, 2, Public) == OK, "reference monday failed");
        Check(Reference.Add(1, 16, 2, ECampusActivity::FreeAccess) == OK && Reference.Add(2, 8, 2, U) == OK && Reference.Add(2, 18, 2, Public) == OK, "reference midweek failed");
        Check(Reference.Add(3, 14, 2, ECampusActivity::Training) == OK && Reference.Add(3, 16, 2, ECampusActivity::FreeAccess) == OK, "reference thursday failed");
        Check(Reference.Add(4, 8, 2, U) == OK && Reference.Add(4, 20, 2, Club) == OK, "reference friday failed");
        Check(Reference.GetBookedHours() == 20 && Reference.GetPreparationHours() == 3, "reference 20+3 hours incorrect");
        // Independent hourly occupancy oracle, checking both insertion orders for every pair.
        for (int A = 0; A < 5; ++A) for (int B = 0; B < 5; ++B)
        for (int StartA = 8; StartA < 22; ++StartA) for (int StartB = 8; StartB < 22; ++StartB)
        {
            const int PrepA = A >= 3 ? 1 : 0, PrepB = B >= 3 ? 1 : 0;
            if (StartA - PrepA < 8 || StartB - PrepB < 8) { continue; }
            bool Occupied[24]{};
            for (int H = StartA - PrepA; H < StartA + 1; ++H) { Occupied[H] = true; }
            bool Overlap = false;
            for (int H = StartB - PrepB; H < StartB + 1; ++H) { Overlap = Overlap || Occupied[H]; }
            FCampusWeeklySchedule Pair;
            Check(Pair.Add(0, StartA, 1, static_cast<ECampusActivity>(A)) == OK, "oracle setup failed");
            Check(Pair.Add(0, StartB, 1, static_cast<ECampusActivity>(B)) == (Overlap ? Busy : OK), "preparation occupancy oracle mismatch");
        }
        std::cout << "PASS: preparation, opening, conflicts both directions, editing, deletion, reference week, occupancy oracle\n";
        std::cout << "PASS: creation, adjacency, overlap, whole duration, opening hours, validation, ids, full week\n";
        std::cout << "PASS: editing, self-overlap, atomic rejection, conflict identity, moving, deletion, reuse, occupancy\n";
        return 0;
    }
    catch (const std::exception& E) { std::cerr << "FAIL: " << E.what() << '\n'; return 1; }
}
