#pragma once
#include "CampusWeeklySchedule.h"
#include <cmath>

enum class ECampusEntry { Activity, MissingStaff, NoContract, NoDemand, FixedCost, Salary, UniversityFee, ContractMissed, RoomBusy, Construction, PathConstruction, Inaccessible, TooLate, PartialContract };
struct FCampusLedgerEntry
{
    std::int64_t Hour = 0;
    ECampusEntry Kind = ECampusEntry::Activity;
    ECampusActivity Activity = ECampusActivity::University;
    int Participants = 0;
    int Revenue = 0;
    int Expense = 0;
    int MinutesAssured = 0;
    int DelayMinutes = 0;
};
struct FCampusWeekResult
{
    std::int64_t Revenue = 0, Expense = 0;
    int UniversityHours = 0, TrainingHours = 0, Completed = 0, Missed = 0;
    int UniversityMinutes = 0, TrainingMinutes = 0;
    std::int64_t Profit() const { return Revenue - Expense; }
};

/** One hall's deterministic execution and ledger. No UI or character dependency. */
class FCampusOperations
{
    friend struct FCampusSaveCodec;
public:
    static constexpr std::int64_t GymCost = 60000;
    std::int64_t Cash = 100000;
    std::int64_t Investment = 0;
    bool PurchaseGym(std::int64_t Minutes)
    {
        if (bGymPurchased || Cash < GymCost || Minutes < 0) { return false; }
        bGymPurchased = true; Cash -= GymCost; Investment += GymCost;
        LastHour = Minutes / 60 - (Minutes % 60 == 0 ? 1 : 0);
        FirstChargeAfterHour = Minutes / 60;
        Entries[NextEntry] = {Minutes / 60, ECampusEntry::Construction, {}, 0, 0, static_cast<int>(GymCost)};
        NextEntry = (NextEntry + 1) % 128; if (Count < 128) { ++Count; }
        return true;
    }
    // Grid.Apply has already debited the price atomically with the tiles.
    void RecordPathInvestment(int Price, std::int64_t Minutes)
    {
        if(Price<=0) { return; }
        Investment+=Price;
        Entries[NextEntry]={Minutes/60,ECampusEntry::PathConstruction,{},0,0,Price};
        NextEntry=(NextEntry+1)%128; if(Count<128) { ++Count; }
    }
    void SetAccess(bool Connected,double Metres,double ReadyMinute)
    {
        bConnected=Connected;
        TravelMinutes=Metres>0 ? static_cast<int>(std::ceil(Metres/72.)) : 0;
        AccessReadyMinute=ReadyMinute;
    }
    const FCampusReservation& ActiveReservation() const { return Active; }
    bool ActiveCanRun() const { return bActive && Outcome==ECampusEntry::Activity; }
    int ActiveParticipants() const { return ActiveCanRun() ? Participants : 0; }
    double ActiveStartMinute() const { return (ActiveEnd-Active.Duration)*60.; }
    double ActiveArrivalMinute() const { return ActiveStartMinute()+Delay; }
    int ActiveDelayMinutes() const { return Delay; }
    bool GroupAlreadyHere(ECampusGroup Group,std::int64_t Day) const { return Group!=ECampusGroup::None && GroupArrivalDay[static_cast<int>(Group)]==Day; }
    FCampusWeekResult Week, PreviousWeek;
    bool HasPreviousWeek = false;
    int ContractState = 0; // 0 offer, 1 accepted, 2 honored, 3 missed
    std::int64_t ContractWeek = -1;
    std::int64_t GetHour() const { return LastHour; }
    std::int64_t OfferedWeek() const { return LastHour / 168 + (LastHour % 168 >= 116 ? 1 : 0); }
    bool AcceptContract()
    {
        if (ContractState == 1 || ContractWeek == OfferedWeek()) { return false; }
        ContractWeek = OfferedWeek(); ContractState = 1; return true;
    }
    bool IsActive() const { return bActive; }
    std::int64_t ActiveId() const { return bActive ? Active.Id : 0; }
    bool StartedThisWeek(std::int64_t Id) const
    {
        for (int I = 0; I < StartedCount; ++I) { if (StartedIds[I] == Id) { return true; } }
        return false;
    }
    int EntryCount() const { return Count; }
    const FCampusLedgerEntry& RecentEntry(int Offset) const { return Entries[(NextEntry + 128 - 1 - Offset) % 128]; }
    static int PublicParticipants(int Start, int Price)
    {
        if (Price < 0 || Price > 30) { return 0; }
        const int Base = Start < 16 ? 8 : Start < 18 ? 12 : 20;
        const int Demand = Base * (160 - 5 * Price) / 100;
        return Demand < 20 ? Demand : 20;
    }
    void AdvanceTo(std::int64_t Hour, const FCampusWeeklySchedule& Schedule)
    {
        // Repeated calls and pause do not post duplicate entries.
        while (LastHour < Hour)
        {
            ++LastHour;
            if (bActive && LastHour == ActiveEnd) { Finish(); }
            if (LastHour % 24 == 0 && LastHour > FirstChargeAfterHour)
            {
                Post({LastHour, ECampusEntry::FixedCost, {}, 0, 0, 100});
                if (LastHour % 168 == 0)
                {
                    if (Schedule.IsStaffHired()) { Post({LastHour, ECampusEntry::Salary, {}, 0, 0, 600}); }
                    Post({LastHour, ECampusEntry::UniversityFee, {}, 0, 1200, 0});
                    PreviousWeek = Week; HasPreviousWeek = true; Week = {};
                    StartedCount = 0;
                }
            }
            if (ContractState == 1 && LastHour >= ContractWeek * 168 + 118)
            {
                ContractState = 3;
                Post({LastHour, ECampusEntry::ContractMissed});
            }
            const int Day = static_cast<int>((LastHour / 24) % 7);
            const int Time = static_cast<int>(LastHour % 24);
            const auto* R = Schedule.FindAt(Day, Time);
            if (R && R->StartHour == Time && !StartedThisWeek(R->Id))
            {
                StartedIds[StartedCount++] = R->Id;
                if (bActive)
                {
                    ++Week.Missed; Post({LastHour, ECampusEntry::RoomBusy, R->Activity});
                    continue;
                }
                // Snapshot: editing the recurring template never rewrites a started occurrence.
                Active = *R; ActiveEnd = LastHour + R->Duration; bActive = true;
                Outcome = ECampusEntry::Activity;
                Delay=0;
                if(!GroupAlreadyHere(R->Group,LastHour/24))
                {
                    const double Earliest=(AccessReadyMinute>R->PlannedAtMinute ? AccessReadyMinute : static_cast<double>(R->PlannedAtMinute))+TravelMinutes;
                    Delay=Earliest>LastHour*60. ? static_cast<int>(std::ceil(Earliest-LastHour*60.)) : 0;
                }
                AssuredMinutes=R->Duration*60-Delay;
                if(AssuredMinutes<0) { AssuredMinutes=0; }
                Participants = R->Activity == ECampusActivity::University ? 24 : R->Activity == ECampusActivity::Training ? 16
                    : R->Activity == ECampusActivity::PublicLesson ? PublicParticipants(Time, R->Price)
                    : R->Activity == ECampusActivity::ClubRental ? 20 : Time < 16 ? 12 : 24;
                if (R->Activity == ECampusActivity::FreeAccess && R->Group == ECampusGroup::Team) { Participants = 16; }
                if (R->Activity == ECampusActivity::FreeAccess && R->Group == ECampusGroup::Students) { Participants = 24; }
                if(!bConnected) { Outcome=ECampusEntry::Inaccessible; }
                else if (CampusNeedsStaff(R->Activity) && (!Schedule.IsStaffHired() || R->StaffId != 1 || Day > 4 || R->EndHour() > 20))
                { Outcome = ECampusEntry::MissingStaff; }
                else if (R->Activity == ECampusActivity::ClubRental && (!R->Contract || ContractState != 1 || ContractWeek != LastHour / 168))
                { Outcome = ECampusEntry::NoContract; }
                else if (Participants == 0) { Outcome = ECampusEntry::NoDemand; }
                else if(AssuredMinutes==0) { Outcome=ECampusEntry::TooLate; }
            }
        }
    }
private:
    std::int64_t LastHour = 7;
    bool bGymPurchased=false, bConnected=true;
    int TravelMinutes=0, Delay=0, AssuredMinutes=0;
    double AccessReadyMinute=0;
    std::int64_t GroupArrivalDay[3]={-1,-1,-1};
    std::int64_t FirstChargeAfterHour = -1;
    FCampusReservation Active{};
    std::int64_t ActiveEnd = 0;
    bool bActive = false;
    int Participants = 0;
    ECampusEntry Outcome = ECampusEntry::Activity;
    FCampusLedgerEntry Entries[128]{};
    int Count = 0, NextEntry = 0;
    std::int64_t StartedIds[168]{};
    int StartedCount = 0;
    void Post(const FCampusLedgerEntry& Entry)
    {
        Cash += Entry.Revenue - Entry.Expense;
        Week.Revenue += Entry.Revenue; Week.Expense += Entry.Expense;
        Entries[NextEntry] = Entry; NextEntry = (NextEntry + 1) % 128;
        if (Count < 128) { ++Count; }
    }
    void Finish()
    {
        bActive = false;
        if (Outcome != ECampusEntry::Activity)
        {
            ++Week.Missed;
            Post({LastHour, Outcome, Active.Activity,0,0,0,0,Delay});
            return;
        }
        const bool PartialClub=Active.Activity==ECampusActivity::ClubRental && Delay>0;
        const int Revenue = Active.Activity == ECampusActivity::PublicLesson ? Active.Price * Participants
            : Active.Activity == ECampusActivity::ClubRental && !PartialClub ? 300 : 0;
        if (Active.Activity == ECampusActivity::ClubRental) { ContractState = PartialClub ? 3 : 2; }
        if(Active.Group!=ECampusGroup::None) { GroupArrivalDay[static_cast<int>(Active.Group)]=LastHour/24; }
        if (Active.Activity == ECampusActivity::University) { Week.UniversityMinutes+=AssuredMinutes; Week.UniversityHours=Week.UniversityMinutes/60; }
        if (Active.Activity == ECampusActivity::Training) { Week.TrainingMinutes+=AssuredMinutes; Week.TrainingHours=Week.TrainingMinutes/60; }
        ++Week.Completed;
        Post({LastHour, PartialClub ? ECampusEntry::PartialContract : ECampusEntry::Activity, Active.Activity, Participants, Revenue, (AssuredMinutes*10+30)/60,AssuredMinutes,Delay});
    }
};
