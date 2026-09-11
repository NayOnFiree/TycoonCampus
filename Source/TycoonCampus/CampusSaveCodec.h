#pragma once
#include "CampusSaveState.h"
#include "CampusConstruction.h"
#include <cstring>
#include <type_traits>

// Versioned, little-endian fields. Never write C++ object memory, padding or pointers.
struct FCampusSaveCodec
{
    static constexpr std::size_t MaxBytes=1024*1024;
    static constexpr std::uint32_t Magic=0x5443414D, Version=1;
    struct Writer
    {
        unsigned char* Bytes; std::size_t Capacity, Position=0; bool Good=true;
        template<class T> void operator()(T& Value)
        {
            using V=std::remove_cv_t<T>;
            if constexpr(std::is_enum_v<V>) { int N=static_cast<int>(Value); (*this)(N); }
            else
            {
                const std::size_t N=std::is_same_v<V,bool>?1:sizeof(V);
                if(Position+N>Capacity) { Good=false; return; }
                std::uint64_t Bits=0;
                if constexpr(std::is_floating_point_v<V>) { std::memcpy(&Bits,&Value,N); }
                else { Bits=static_cast<std::uint64_t>(Value); }
                for(std::size_t I=0;I<N;++I) { Bytes[Position++]=static_cast<unsigned char>(Bits>>(8*I)); }
            }
        }
    };
    struct Reader
    {
        const unsigned char* Bytes; std::size_t Capacity, Position=0; bool Good=true;
        template<class T> void operator()(T& Value)
        {
            if constexpr(std::is_enum_v<T>) { int N=0; (*this)(N); Value=static_cast<T>(N); }
            else
            {
                const std::size_t N=std::is_same_v<T,bool>?1:sizeof(T);
                if(Position+N>Capacity) { Good=false; return; }
                std::uint64_t Bits=0;
                for(std::size_t I=0;I<N;++I) { Bits|=static_cast<std::uint64_t>(Bytes[Position++])<<(8*I); }
                if constexpr(std::is_same_v<T,bool>) { if(Bits>1) { Good=false; } Value=Bits!=0; }
                else if constexpr(std::is_floating_point_v<T>) { std::memcpy(&Value,&Bits,N); }
                else { Value=static_cast<T>(Bits); }
            }
        }
    };
    static std::uint64_t Hash(const unsigned char* Bytes,std::size_t Size)
    {
        std::uint64_t H=14695981039346656037ull;
        for(std::size_t I=0;I<Size;++I) { H=(H^Bytes[I])*1099511628211ull; }
        return H;
    }
    template<class A> static void Reservation(A& Ar,FCampusReservation& R)
    { Ar(R.Id); Ar(R.Day); Ar(R.StartHour); Ar(R.Duration); Ar(R.Activity); Ar(R.Group); Ar(R.StaffId); Ar(R.Price); Ar(R.Contract); Ar(R.PlannedAtMinute); }
    template<class A> static void Week(A& Ar,FCampusWeekResult& W)
    { Ar(W.Revenue); Ar(W.Expense); Ar(W.UniversityHours); Ar(W.TrainingHours); Ar(W.Completed); Ar(W.Missed); Ar(W.UniversityMinutes); Ar(W.TrainingMinutes); }
    template<class A> static void Fields(A& Ar,FCampusSaveState& S)
    {
        Ar(S.Clock.Seconds); Ar(S.Clock.RoundingCorrection); Ar(S.Clock.bPaused); Ar(S.Clock.Speed);
        Ar(S.Built); Ar(S.Selected); Ar(S.BuildingX); Ar(S.BuildingY); Ar(S.Turns);
        for(auto& C:S.Paths) { Ar(C); } Ar(S.PathReadyMinute);
        Ar(S.CameraX); Ar(S.CameraY); Ar(S.CameraZoom); Ar(S.CameraYaw); Ar(S.CameraPitch);
        auto& P=S.Schedule;
        Ar(P.ReservationCount); Ar(P.NextId); Ar(P.bStaffHired);
        for(auto& R:P.Reservations) { Reservation(Ar,R); }
        auto& O=S.Operations;
        Ar(O.Cash); Ar(O.Investment); Week(Ar,O.Week); Week(Ar,O.PreviousWeek); Ar(O.HasPreviousWeek);
        Ar(O.ContractState); Ar(O.ContractWeek); Ar(O.LastHour); Ar(O.bGymPurchased); Ar(O.bConnected);
        Ar(O.TravelMinutes); Ar(O.Delay); Ar(O.AssuredMinutes); Ar(O.AccessReadyMinute);
        for(auto& D:O.GroupArrivalDay) { Ar(D); }
        Ar(O.FirstChargeAfterHour); Reservation(Ar,O.Active); Ar(O.ActiveEnd); Ar(O.bActive); Ar(O.Participants); Ar(O.Outcome);
        Ar(O.Count); Ar(O.NextEntry);
        for(auto& E:O.Entries)
        { Ar(E.Hour); Ar(E.Kind); Ar(E.Activity); Ar(E.Participants); Ar(E.Revenue); Ar(E.Expense); Ar(E.MinutesAssured); Ar(E.DelayMinutes); }
        Ar(O.StartedCount); for(auto& Id:O.StartedIds) { Ar(Id); }
        auto& V=S.Visual;
        Ar(V.Id); Ar(V.Day); Ar(V.Started); Ar(V.People); Reservation(Ar,V.Reservation);
        Ar(V.Start); Ar(V.Arrival); Ar(V.End); Ar(V.Travel); Ar(V.Animation); Ar(V.RouteCount);
        if(V.RouteCount<0 || V.RouteCount>10001) { Ar.Good=false; return; }
        for(int I=0;I<V.RouteCount;++I) { Ar(V.Route[I].X); Ar(V.Route[I].Y); Ar(V.Route[I].Z); }
    }
    static bool Range(double V,double Min,double Max) { return std::isfinite(V) && V>=Min && V<=Max; }
    static bool ValidReservation(const FCampusReservation& R,std::int64_t Now)
    {
        return R.Id>0 && R.Id<1000000000 && R.Day>=0 && R.Day<7 && R.StartHour>=8 && R.StartHour<22 && R.Duration>=1 && R.Duration<=14
            && static_cast<int>(R.Activity)>=0 && static_cast<int>(R.Activity)<=4 && CampusGroupCompatible(R.Activity,R.Group)
            && R.StartHour-CampusPreparationHours(R.Activity)>=8 && R.EndHour()<=22 && R.StaffId>=0 && R.StaffId<=1 && R.Price>=0 && R.Price<=30
            && R.PlannedAtMinute>=0 && R.PlannedAtMinute<=Now;
    }
    static bool ValidWeek(const FCampusWeekResult& W)
    {
        return W.Revenue>=0 && W.Revenue<=1000000000 && W.Expense>=0 && W.Expense<=1000000000
            && W.Completed>=0 && W.Completed<=168 && W.Missed>=0 && W.Missed<=168
            && W.UniversityMinutes>=0 && W.UniversityMinutes<=10080 && W.TrainingMinutes>=0 && W.TrainingMinutes<=10080
            && W.UniversityHours==W.UniversityMinutes/60 && W.TrainingHours==W.TrainingMinutes/60;
    }
    static bool Validate(const FCampusSaveState& S)
    {
        if(!Range(S.Clock.Seconds,28800,1e10) || !Range(S.Clock.RoundingCorrection,-1,1) || (S.Clock.Speed!=1 && S.Clock.Speed!=2 && S.Clock.Speed!=4)) { return false; }
        const auto Now=S.Clock.GetTotalMinutes(), Hour=Now/60;
        if(!Range(S.BuildingX,-10000,10000) || !Range(S.BuildingY,-10000,10000) || S.Turns<0 || S.Turns>3
            || !Range(S.PathReadyMinute,0,static_cast<double>(Now)) || !Range(S.CameraX,-9000,9000) || !Range(S.CameraY,-9000,9000)
            || !Range(S.CameraZoom,2500,24000) || !Range(S.CameraYaw,-360,360) || !Range(S.CameraPitch,-80,-25)) { return false; }
        if(S.Built)
        {
            const auto F=FCampusFootprint::AtCursor(S.BuildingX,S.BuildingY,S.Turns%2!=0);
            if(!F.InsideParcel() || std::abs(F.CenterX()-S.BuildingX)>.01 || std::abs(F.CenterY()-S.BuildingY)>.01) { return false; }
        }
        for(auto C:S.Paths) { if(C>2) { return false; } }
        const auto& P=S.Schedule;
        if(P.ReservationCount<0 || P.ReservationCount>98 || P.NextId<1 || P.NextId>=1000000000) { return false; }
        FCampusWeeklySchedule Check;
        if(P.bStaffHired) { Check.HireStaff(); }
        for(int I=0;I<P.ReservationCount;++I)
        {
            const auto& R=P.Reservations[I];
            if(!ValidReservation(R,Now) || R.Id>=P.NextId) { return false; }
            for(int J=0;J<I;++J) { if(P.Reservations[J].Id==R.Id) { return false; } }
            if(Check.Add(R.Day,R.StartHour,R.Duration,R.Activity,static_cast<int>(R.Group),R.StaffId,R.Price,R.Contract,R.PlannedAtMinute)!=ECampusBookingResult::Success) { return false; }
        }
        const auto& O=S.Operations;
        if(O.Cash < -1000000000000ll || O.Cash>1000000000000ll || O.Investment<0 || O.Investment>1000000000000ll || O.bGymPurchased!=S.Built
            || (S.Built && O.Investment<60000) || !ValidWeek(O.Week) || !ValidWeek(O.PreviousWeek)
            || O.LastHour<7 || O.LastHour>Hour || (S.Built && Hour-O.LastHour>168)
            || O.ContractState<0 || O.ContractState>3 || O.ContractWeek < -1 || O.ContractWeek>Hour/168+1
            || (O.ContractState==0 && O.ContractWeek!=-1) || (O.ContractState!=0 && O.ContractWeek<0)
            || O.Count<0 || O.Count>128 || O.NextEntry<0 || O.NextEntry>=128 || (O.Count<128 && O.NextEntry!=O.Count)
            || O.StartedCount<0 || O.StartedCount>168 || O.TravelMinutes<0 || O.TravelMinutes>100000 || O.Delay<0 || O.Delay>100000
            || O.AssuredMinutes<0 || O.AssuredMinutes>840 || O.Participants<0 || O.Participants>30
            || !Range(O.AccessReadyMinute,0,static_cast<double>(Now)) || O.FirstChargeAfterHour < -1 || O.FirstChargeAfterHour>Hour) { return false; }
        for(auto D:O.GroupArrivalDay) { if(D < -1 || D>Now/1440) { return false; } }
        for(int I=0;I<O.StartedCount;++I)
        {
            if(O.StartedIds[I]<=0 || O.StartedIds[I]>=P.NextId) { return false; }
            for(int J=0;J<I;++J) { if(O.StartedIds[J]==O.StartedIds[I]) { return false; } }
        }
        if(O.bActive)
        {
            if(!S.Built || !ValidReservation(O.Active,Now) || O.Active.Id>=P.NextId || O.ActiveEnd<=O.LastHour || O.ActiveEnd-O.Active.Duration>O.LastHour
                || O.Active.StartHour!=(O.ActiveEnd-O.Active.Duration)%24 || O.Active.Day!=((O.ActiveEnd-O.Active.Duration)/24)%7 || !O.StartedThisWeek(O.Active.Id)) { return false; }
            const auto K=O.Outcome;
            if(K!=ECampusEntry::Activity && K!=ECampusEntry::MissingStaff && K!=ECampusEntry::NoContract && K!=ECampusEntry::NoDemand && K!=ECampusEntry::Inaccessible && K!=ECampusEntry::TooLate) { return false; }
        }
        for(int I=0;I<O.Count;++I)
        {
            const auto& E=O.RecentEntry(I);
            if(E.Hour<0 || E.Hour>Hour || static_cast<int>(E.Kind)<0 || static_cast<int>(E.Kind)>13 || static_cast<int>(E.Activity)<0 || static_cast<int>(E.Activity)>4
                || E.Participants<0 || E.Participants>30 || E.Revenue<0 || E.Revenue>1000000000 || E.Expense<0 || E.Expense>1000000000
                || E.MinutesAssured<0 || E.MinutesAssured>840 || E.DelayMinutes<0 || E.DelayMinutes>100000) { return false; }
        }
        const auto& V=S.Visual;
        if(V.Id<0 || V.Id>=P.NextId || V.Day < -1 || V.Day>Now/1440 || V.People<0 || V.People>8 || V.RouteCount<0 || V.RouteCount>10001
            || !Range(V.Start,0,Now+1440.) || !Range(V.Arrival,0,Now+100000.) || !Range(V.End,0,Now+1440.)
            || !Range(V.Travel,0,100000) || !Range(V.Animation,0,1e10) || (V.Id && !ValidReservation(V.Reservation,Now))) { return false; }
        for(int I=0;I<V.RouteCount;++I)
        { const auto& Pt=V.Route[I]; if(!Range(Pt.X,-10000,10000) || !Range(Pt.Y,-10000,10000) || !Range(Pt.Z,0,850)) { return false; } }
        return true;
    }
    static bool Encode(FCampusSaveState& S,unsigned char* Bytes,std::size_t Capacity,std::size_t& Size)
    {
        Size=0; if(!Validate(S)) { return false; }
        Writer W{Bytes,Capacity}; auto M=Magic,V=Version; W(M); W(V); Fields(W,S);
        if(!W.Good) { return false; } auto H=Hash(Bytes,W.Position); W(H);
        if(!W.Good) { return false; } Size=W.Position; return true;
    }
    // Decode into a temporary state, validate, then the caller commits it to the world.
    static bool Decode(const unsigned char* Bytes,std::size_t Size,FCampusSaveState& S)
    {
        if(Size<16 || Size>MaxBytes) { return false; }
        Reader Tail{Bytes+Size-8,8}; std::uint64_t Expected=0; Tail(Expected);
        if(Hash(Bytes,Size-8)!=Expected) { return false; }
        Reader R{Bytes,Size-8}; std::uint32_t M=0,V=0; R(M); R(V);
        if(M!=Magic || V!=Version) { return false; }
        Fields(R,S); S.Clock.bAdvancing=false;
        return R.Good && R.Position==R.Capacity && Validate(S);
    }
};
