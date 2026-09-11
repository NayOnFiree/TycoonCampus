#include "../Source/TycoonCampus/CampusSaveCodec.h"
#include <memory>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <limits>
static void Check(bool B,const char* M) { if(!B) { throw std::runtime_error(M); } }
static std::vector<unsigned char> Encode(FCampusSaveState& S)
{
    std::vector<unsigned char> Bytes(FCampusSaveCodec::MaxBytes); std::size_t Size=0;
    Check(FCampusSaveCodec::Encode(S,Bytes.data(),Bytes.size(),Size),"encode failed"); Bytes.resize(Size); return Bytes;
}
static std::unique_ptr<FCampusSaveState> RoundTrip(FCampusSaveState& S)
{
    const auto Bytes=Encode(S); auto R=std::make_unique<FCampusSaveState>();
    Check(FCampusSaveCodec::Decode(Bytes.data(),Bytes.size(),*R),"decode failed");
    Check(Bytes==Encode(*R),"roundtrip not exact"); return R;
}
static void Advance(FCampusSaveState& S,double Hour)
{
    S.Clock.SetSpeed(1);
    S.Clock.Advance((Hour*3600.-S.Clock.GetTotalSeconds())/288.,[](auto){});
    if(S.Built) { S.Operations.AdvanceTo(S.Clock.GetTotalMinutes()/60,S.Schedule); }
}
static void Rehash(std::vector<unsigned char>& B)
{
    auto H=FCampusSaveCodec::Hash(B.data(),B.size()-8);
    FCampusSaveCodec::Writer W{B.data()+B.size()-8,8}; W(H);
}
int main()
{
    try
    {
        auto S=std::make_unique<FCampusSaveState>();
        S->Paths[123]=2; S->Operations.Cash-=20; S->Operations.RecordPathInvestment(20,480);
        RoundTrip(*S); Advance(*S,240); RoundTrip(*S); // unbuilt campus can run for weeks
        S=std::make_unique<FCampusSaveState>();
        S->Built=true; Check(S->Operations.PurchaseGym(480),"purchase");
        const auto F=FCampusFootprint::AtCursor(700,0,false); S->BuildingX=F.CenterX(); S->BuildingY=F.CenterY();
        S->Schedule.HireStaff();
        Check(S->Schedule.Add(0,18,2,ECampusActivity::PublicLesson,-1,1)==ECampusBookingResult::Success,"public booking");
        Check(S->Schedule.Add(4,20,2,ECampusActivity::ClubRental,-1,0,12,true)==ECampusBookingResult::Success,"club booking");
        S->Operations.AcceptContract(); S->Operations.SetAccess(true,144,480);
        Advance(*S,18.5); Check(S->Operations.IsActive(),"active session");
        S->Visual.Id=1; S->Visual.Day=0; S->Visual.Started=true; S->Visual.Reservation=S->Operations.ActiveReservation();
        S->Visual.Start=1080; S->Visual.End=1200; S->Visual.Arrival=1080; S->Visual.Travel=2;
        S->Visual.RouteCount=2; S->Visual.Route[0]={-9900,100,20}; S->Visual.Route[1]={S->BuildingX,S->BuildingY,30};
        S->CameraZoom=5000; S->CameraYaw=179; S->CameraPitch=-70;
        auto R=RoundTrip(*S);
        S->Schedule.Remove(1); R->Schedule.Remove(1); // active snapshot survives deletion
        R=RoundTrip(*R);
        Advance(*S,20); Advance(*R,20);
        Check(S->Operations.Cash==40220,"expected public net");
        Check(Encode(*S)==Encode(*R),"resumed payment differs");
        R=RoundTrip(*R); auto Cash=R->Operations.Cash; R->Operations.AdvanceTo(20,R->Schedule);
        Check(R->Operations.Cash==Cash,"repeated load duplicated payment");
        for(double Hour:{23.99,24.,115.99,116.5,118.,167.99,168.,336.,672.,4000.})
        {
            Advance(*S,Hour); Advance(*R,Hour); R=RoundTrip(*R);
            Check(Encode(*S)==Encode(*R),"midnight/week/semester continuation differs");
        }
        Check(R->Operations.EntryCount()==128,"ledger ring wrap not exercised");
        for(bool Connected:{false,true})
        {
            auto D=std::make_unique<FCampusSaveState>(); D->Built=true; D->BuildingX=F.CenterX(); D->BuildingY=F.CenterY();
            D->Operations.PurchaseGym(480); D->Schedule.Add(0,10,2,ECampusActivity::University);
            D->Operations.SetAccess(Connected,144,600); Advance(*D,10.5); auto E=RoundTrip(*D);
            Advance(*D,12); Advance(*E,12); Check(Encode(*D)==Encode(*E),"missed/delayed session changed");
        }
        auto Bytes=Encode(*S); auto Out=std::make_unique<FCampusSaveState>();
        Check(!FCampusSaveCodec::Decode(Bytes.data(),Bytes.size()-1,*Out),"truncation accepted");
        Bytes[100]^=1; Check(!FCampusSaveCodec::Decode(Bytes.data(),Bytes.size(),*Out),"corruption accepted");
        Bytes=Encode(*S); Bytes[4]=2; Rehash(Bytes);
        Check(!FCampusSaveCodec::Decode(Bytes.data(),Bytes.size(),*Out),"future version accepted");
        Bytes=Encode(*S); Bytes[24]=3; Rehash(Bytes); // invalid bool after clock doubles
        Check(!FCampusSaveCodec::Decode(Bytes.data(),Bytes.size(),*Out),"invalid boolean accepted");
        S->CameraZoom=std::numeric_limits<double>::quiet_NaN(); std::size_t N=0;
        Check(!FCampusSaveCodec::Encode(*S,Bytes.data(),Bytes.size(),N),"NaN accepted");
        S->CameraZoom=9000; S->Paths[4]=9;
        Check(!FCampusSaveCodec::Encode(*S,Bytes.data(),Bytes.size(),N),"bad path accepted");
        std::cout<<"Campus save tests passed: exact roundtrip, active snapshots, payments, calendar, ring ledger, corruption.\n";
        return 0;
    }
    catch(const std::exception& E) { std::cerr<<E.what()<<'\n'; return 1; }
}
