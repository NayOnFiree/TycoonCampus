#pragma once
#include <cstdint>

enum class ECampusActivity { University, Training, FreeAccess, PublicLesson, ClubRental };
inline bool CampusNeedsStaff(ECampusActivity A)
{
    return A == ECampusActivity::University || A == ECampusActivity::Training || A == ECampusActivity::PublicLesson;
}
enum class ECampusGroup { None, Students, Team };
inline ECampusGroup CampusDefaultGroup(ECampusActivity Activity)
{
    return Activity == ECampusActivity::University ? ECampusGroup::Students
        : Activity == ECampusActivity::Training ? ECampusGroup::Team : ECampusGroup::None;
}
inline bool CampusGroupCompatible(ECampusActivity Activity, ECampusGroup Group)
{
    if (Group < ECampusGroup::None || Group > ECampusGroup::Team) { return false; }
    if (Activity == ECampusActivity::FreeAccess) { return true; }
    return Group == CampusDefaultGroup(Activity);
}
struct FCampusGroupNeeds
{
    int Members;
    int RequiredSessions;
    int PlannedHours;
    int PlannedSessions;
    int MissingSessions() const { return PlannedSessions < RequiredSessions ? RequiredSessions - PlannedSessions : 0; }
};
inline int CampusPreparationHours(ECampusActivity Activity)
{
    return Activity == ECampusActivity::PublicLesson || Activity == ECampusActivity::ClubRental ? 1 : 0;
}
enum class ECampusBookingResult { Success, InvalidDay, InvalidStart, InvalidDuration, InvalidActivity, OutsideOpeningHours, Occupied, NotFound, InvalidGroup, InvalidStaff, StaffUnavailable, InvalidPrice, InvalidContract };

struct FCampusReservation
{
    std::int64_t Id;
    int Day;
    int StartHour;
    int Duration;
    ECampusActivity Activity;
    ECampusGroup Group = ECampusGroup::None;
    int StaffId = 0;
    int Price = 12;
    bool Contract = false;
    std::int64_t PlannedAtMinute = 0;
    int EndHour() const { return StartHour + Duration; }
    int PreparationHours() const { return CampusPreparationHours(Activity); }
    int OccupiedStartHour() const { return StartHour - PreparationHours(); }
};

/** A recurring week for one reservable zone. No world/UI lifetime dependency. */
class FCampusWeeklySchedule
{
    friend struct FCampusSaveCodec;
public:
    // One zone has at most 7 * 14 non-overlapping, one-hour bookings.
    static constexpr int MaxReservations = 7 * 14;
    int GetReservationCount() const { return ReservationCount; }
    bool IsStaffHired() const { return bStaffHired; }
    void HireStaff() { bStaffHired = true; }
    int GetStaffHours() const
    {
        int Hours = 0;
        for (int I = 0; I < ReservationCount; ++I) { if (Reservations[I].StaffId == 1) { Hours += Reservations[I].Duration; } }
        return Hours;
    }
    int GetUnstaffedCount() const
    {
        int Count = 0;
        for (int I = 0; I < ReservationCount; ++I)
        { if (CampusNeedsStaff(Reservations[I].Activity) && Reservations[I].StaffId == 0) { ++Count; } }
        return Count;
    }
    bool HasContractBooking() const
    {
        for (int I = 0; I < ReservationCount; ++I) { if (Reservations[I].Contract) { return true; } }
        return false;
    }
    ECampusBookingResult ValidateResources(int Day, int Start, int Duration, ECampusActivity Activity, int StaffId, int Price, bool Contract) const
    {
        if (StaffId < 0 || StaffId > 1 || (StaffId && (!bStaffHired || !CampusNeedsStaff(Activity)))) { return ECampusBookingResult::InvalidStaff; }
        if (StaffId && (Day > 4 || Start + Duration > 20)) { return ECampusBookingResult::StaffUnavailable; }
        if (Price < 0 || Price > 30) { return ECampusBookingResult::InvalidPrice; }
        if (Contract && (Activity != ECampusActivity::ClubRental || Day != 4 || Start != 20 || Duration != 2)) { return ECampusBookingResult::InvalidContract; }
        return ECampusBookingResult::Success;
    }
    FCampusGroupNeeds GetNeeds(ECampusGroup Group) const
    {
        FCampusGroupNeeds Needs{};
        if (Group != ECampusGroup::Students && Group != ECampusGroup::Team) { return Needs; }
        Needs.Members = Group == ECampusGroup::Students ? 24 : 16;
        Needs.RequiredSessions = Group == ECampusGroup::Students ? 3 : 2;
        const auto Activity = Group == ECampusGroup::Students ? ECampusActivity::University : ECampusActivity::Training;
        for (int Index = 0; Index < ReservationCount; ++Index)
        {
            const auto& R = Reservations[Index];
            if (R.Group == Group && R.Activity == Activity)
            {
                Needs.PlannedHours += R.Duration;
                if (R.Duration == 2) { ++Needs.PlannedSessions; }
            }
        }
        return Needs;
    }
    const FCampusReservation* FindAt(int Day, int Hour) const
    {
        for (int Index = 0; Index < ReservationCount; ++Index)
        {
            const auto& R = Reservations[Index];
            if (R.Day == Day && R.OccupiedStartHour() <= Hour && Hour < R.EndHour()) { return &R; }
        }
        return nullptr;
    }

    const FCampusReservation* FindById(std::int64_t Id) const
    {
        for (int Index = 0; Index < ReservationCount; ++Index)
        {
            if (Reservations[Index].Id == Id) { return &Reservations[Index]; }
        }
        return nullptr;
    }

    int GetBookedHours() const
    {
        int Hours = 0;
        for (int Index = 0; Index < ReservationCount; ++Index) { Hours += Reservations[Index].Duration; }
        return Hours;
    }

    int GetPreparationHours() const
    {
        int Hours = 0;
        for (int Index = 0; Index < ReservationCount; ++Index) { Hours += Reservations[Index].PreparationHours(); }
        return Hours;
    }

    const FCampusReservation* FindConflict(int Day, int Start, int Duration, std::int64_t IgnoredId = 0,
        ECampusActivity Activity = ECampusActivity::University) const
    {
        // Validate before adding the duration, including calls from the UI.
        if (Day < 0 || Day > 6 || Start < 8 || Start >= 22 || Duration < 1 || Duration > 14) { return nullptr; }
        for (int Index = 0; Index < ReservationCount; ++Index)
        {
            const auto& R = Reservations[Index];
            if (R.Id != IgnoredId && R.Day == Day && Start - CampusPreparationHours(Activity) < R.EndHour()
                && R.OccupiedStartHour() < Start + Duration) { return &R; }
        }
        return nullptr;
    }

    ECampusBookingResult Validate(int Day, int Start, int Duration, ECampusActivity Activity, std::int64_t IgnoredId = 0) const
    {
        if (Day < 0 || Day > 6) { return ECampusBookingResult::InvalidDay; }
        if (Start < 8 || Start >= 22) { return ECampusBookingResult::InvalidStart; }
        if (Duration < 1 || Duration > 14) { return ECampusBookingResult::InvalidDuration; }
        if (static_cast<int>(Activity) < 0 || static_cast<int>(Activity) > 4) { return ECampusBookingResult::InvalidActivity; }
        if (Start - CampusPreparationHours(Activity) < 8 || Start + Duration > 22) { return ECampusBookingResult::OutsideOpeningHours; }
        if (FindConflict(Day, Start, Duration, IgnoredId, Activity)) { return ECampusBookingResult::Occupied; }
        return ECampusBookingResult::Success;
    }

    ECampusBookingResult Add(int Day, int Start, int Duration, ECampusActivity Activity, int GroupId = -1, int StaffId = 0, int Price = 12, bool Contract = false, std::int64_t PlannedAt = 0)
    {
        const auto Group = GroupId == -1 ? CampusDefaultGroup(Activity) : static_cast<ECampusGroup>(GroupId);
        if (!CampusGroupCompatible(Activity, Group)) { return ECampusBookingResult::InvalidGroup; }
        const auto Result = Validate(Day, Start, Duration, Activity);
        if (Result != ECampusBookingResult::Success) { return Result; }
        const auto Resources = ValidateResources(Day, Start, Duration, Activity, StaffId, Price, Contract);
        if (Resources != ECampusBookingResult::Success) { return Resources; }
        if (ReservationCount >= MaxReservations) { return ECampusBookingResult::Occupied; }
        Reservations[ReservationCount++] = {NextId++, Day, Start, Duration, Activity, Group, StaffId, Price, Contract, PlannedAt};
        return ECampusBookingResult::Success;
    }

    ECampusBookingResult Update(std::int64_t Id, int Day, int Start, int Duration, ECampusActivity Activity, int GroupId = -1, int StaffId = 0, int Price = 12, bool Contract = false, std::int64_t PlannedAt = 0)
    {
        if (!FindById(Id)) { return ECampusBookingResult::NotFound; }
        const auto Group = GroupId == -1 ? CampusDefaultGroup(Activity) : static_cast<ECampusGroup>(GroupId);
        if (!CampusGroupCompatible(Activity, Group)) { return ECampusBookingResult::InvalidGroup; }
        const auto Result = Validate(Day, Start, Duration, Activity, Id);
        if (Result != ECampusBookingResult::Success) { return Result; }
        const auto Resources = ValidateResources(Day, Start, Duration, Activity, StaffId, Price, Contract);
        if (Resources != ECampusBookingResult::Success) { return Resources; }
        for (int Index = 0; Index < ReservationCount; ++Index)
        {
            if (Reservations[Index].Id == Id)
            {
                const auto& Old=Reservations[Index];
                const auto Departure = Old.Day==Day && Old.StartHour==Start && Old.Activity==Activity && Old.Group==Group ? Old.PlannedAtMinute : PlannedAt;
                Reservations[Index] = {Id, Day, Start, Duration, Activity, Group, StaffId, Price, Contract, Departure};
                return ECampusBookingResult::Success;
            }
        }
        return ECampusBookingResult::NotFound;
    }

    bool Remove(std::int64_t Id)
    {
        for (int Index = 0; Index < ReservationCount; ++Index)
        {
            if (Reservations[Index].Id != Id) { continue; }
            for (int Next = Index + 1; Next < ReservationCount; ++Next) { Reservations[Next - 1] = Reservations[Next]; }
            Reservations[--ReservationCount] = {};
            return true;
        }
        return false;
    }

private:
    FCampusReservation Reservations[MaxReservations]{};
    int ReservationCount = 0;
    std::int64_t NextId = 1;
    bool bStaffHired = false;
};
