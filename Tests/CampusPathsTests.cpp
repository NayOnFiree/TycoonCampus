#include "../Source/TycoonCampus/CampusPathGrid.h"
#include "../Source/TycoonCampus/CampusOperations.h"
#include <iostream>
#include <stdexcept>
static void Check(bool V,const char* M) { if(!V) { throw std::runtime_error(M); } }
int main()
{
    try
    {
        FCampusPathGrid G; std::int64_t Cash=1000;
        Check(G.Cell(-10000)==0 && G.Cell(9999)==99 && G.Cell(10000)==-1,"grid bounds");
        Check(G.MakeSegment(0,0,2,2,false) && G.SegmentCount==5 && G.Quote(false)==100,"L quote");
        Check(G.Apply(false,Cash) && Cash==900,"path purchase");
        Check(G.Apply(false,Cash) && Cash==900,"duplicate path debit");
        Check(G.FindRoute(0,202) && G.RouteCount==5 && G.Route[0]==0 && G.Route[4]==202,"ordered shortest route");
        G.MakeSegment(2,1,2,1,false); Check(G.Apply(true,Cash) && Cash==900 && !G.FindRoute(0,202),"removal does not disconnect");
        G.MakeSegment(0,0,2,2,true); Check(G.Apply(false,Cash) && G.FindRoute(0,202),"alternate route");
        G.Cells[0]=1; G.MakeSegment(0,0,0,0,false); Check(G.Quote(true)==-2 && !G.Apply(true,Cash),"initial path removable");
        G.Blocked[303]=true; G.MakeSegment(2,3,4,3,false); auto Before=Cash;
        Check(G.Quote(false)<0 && !G.Apply(false,Cash) && Cash==Before && !G.Cells[302],"blocked trace partially applied");
        G.MakeSegment(20,20,30,20,false); Cash=1;
        Check(!G.Apply(false,Cash) && Cash==1 && !G.Cells[2020],"unaffordable trace mutated");
        Check(!G.MakeSegment(-1,0,0,0,false) && G.Quote(false)<0,"invalid trace");
        FCampusPathGrid Diagonal; Diagonal.Cells[0]=2; Diagonal.Cells[101]=2;
        Check(!Diagonal.FindRoute(0,101),"diagonal falsely connected");
        Check(!Diagonal.FindRoute(-1,101),"invalid endpoint");

        static FCampusPathGrid Rectangle;
        Cash=1000;
        Check(Rectangle.MakeRectangle(0,0,0,0) && Rectangle.SegmentCount==4 && Rectangle.Quote(false)==80,"4m tile size and quote");
        Check(Rectangle.Apply(false,Cash) && Cash==920,"4m tile purchase");
        Check(Rectangle.MakeRectangle(3,5,0,0) && Rectangle.SegmentCount==24 && Rectangle.Quote(false)==400,"reverse rectangle and existing tiles");
        Check(Rectangle.Apply(false,Cash) && Cash==520 && Rectangle.FindRoute(0,503),"filled rectangle connection");
        Check(Rectangle.MakeRectangle(0,5,3,0) && Rectangle.SegmentCount==24 && Rectangle.Quote(false)==0,"opposite drag quadrant");
        Check(Rectangle.Apply(true,Cash) && Cash==520 && !Rectangle.Cells[503],"rectangle removal");
        Rectangle.Blocked[101]=true;
        Rectangle.MakeRectangle(0,0,2,2);
        Check(!Rectangle.Apply(false,Cash) && Cash==520 && !Rectangle.Cells[0],"blocked rectangle atomicity");
        Rectangle.Blocked[101]=false; Cash=1;
        Check(!Rectangle.Apply(false,Cash) && Cash==1 && !Rectangle.Cells[0],"rectangle affordability atomicity");
        Rectangle.Cells[101]=1;
        Check(Rectangle.Quote(true)==-2 && !Rectangle.Apply(true,Cash) && Rectangle.Cells[101]==1,"rectangle protects entrance");
        Check(Rectangle.MakeRectangle(99,99,99,99) && Rectangle.SegmentCount==4 && Rectangle.Segment[3]==9999,"last coarse tile bounds");
        Check(Rectangle.MakeRectangle(99,99,0,0) && Rectangle.SegmentCount==10000 && Rectangle.Segment[9999]==9999,"full parcel selection capacity");
        Check(!Rectangle.MakeRectangle(-1,0,0,0) && Rectangle.SegmentCount==0,"out of bounds rectangle clears preview");

        Check(Rectangle.MakeRectangle(9,9,9,9,1) && Rectangle.SegmentCount==1 && Rectangle.Segment[0]==909,"precision for legacy narrow connections");
        Check(!Rectangle.MakeRectangle(0,0,1,1,0),"invalid stride");
        FCampusWeeklySchedule S; S.HireStaff();
        Check(S.Add(0,8,2,ECampusActivity::University,-1,1)==ECampusBookingResult::Success,"schedule setup");
        FCampusOperations Closed; Closed.SetAccess(false,100,480); Closed.AdvanceTo(10,S);
        Check(Closed.Week.Missed==1 && Closed.Week.UniversityMinutes==0 && Closed.Cash==100000 && Closed.RecentEntry(0).Kind==ECampusEntry::Inaccessible,"inaccessible activity executed");
        FCampusOperations Late; Late.SetAccess(true,144,480); Late.AdvanceTo(10,S);
        Check(Late.Week.UniversityMinutes==118 && Late.RecentEntry(0).DelayMinutes==2 && Late.RecentEntry(0).MinutesAssured==118,"late minutes incorrect");
        FCampusOperations Early; Early.SetAccess(true,144,470); Early.AdvanceTo(10,S);
        Check(Early.Week.UniversityMinutes==120 && Early.Week.Expense==20,"anticipated arrival penalized");
        FCampusWeeklySchedule LastMinute; LastMinute.HireStaff();
        LastMinute.Add(0,9,2,ECampusActivity::University,-1,1,12,false,539);
        FCampusOperations ShortNotice; ShortNotice.SetAccess(true,144,480); ShortNotice.AdvanceTo(11,LastMinute);
        Check(ShortNotice.Week.UniversityMinutes==119 && ShortNotice.RecentEntry(0).DelayMinutes==1,"last-minute booking departed before creation");
        FCampusOperations Far; Far.SetAccess(true,9000,480); Far.AdvanceTo(10,S);
        Check(Far.Week.Missed==1 && Far.RecentEntry(0).Kind==ECampusEntry::TooLate && Far.Cash==100000,"arrival after end executed");
        FCampusOperations Repaired; Repaired.SetAccess(false,0,480); Repaired.AdvanceTo(8,S);
        Repaired.SetAccess(true,144,481); Repaired.AdvanceTo(10,S);
        Check(Repaired.Week.Missed==1,"repair retroactively rescued started occurrence");
        FCampusOperations Interrupted; Interrupted.SetAccess(true,144,470); Interrupted.AdvanceTo(8,S);
        Interrupted.SetAccess(false,0,490); Interrupted.AdvanceTo(10,S);
        Check(Interrupted.Week.UniversityMinutes==120,"active snapshot changed by path deletion");
        S.Add(0,10,2,ECampusActivity::University,-1,1);
        FCampusOperations SameGroup; SameGroup.SetAccess(true,144,480); SameGroup.AdvanceTo(12,S);
        Check(SameGroup.Week.UniversityMinutes==238,"group already at gym walked again");
        FCampusOperations Fast,Slow; Fast.SetAccess(true,144,480); Slow.SetAccess(true,144,480);
        Fast.AdvanceTo(168,S); for(int H=8;H<=168;++H) { Slow.AdvanceTo(H,S); }
        Check(Fast.Cash==Slow.Cash && Fast.PreviousWeek.UniversityMinutes==Slow.PreviousWeek.UniversityMinutes,"access depends on frame batch");
        FCampusWeeklySchedule Club; Club.Add(4,20,2,ECampusActivity::ClubRental,-1,0,12,true);
        FCampusOperations Partial; Partial.AcceptContract(); Partial.SetAccess(true,144,116*60); Partial.AdvanceTo(118,Club);
        Check(Partial.ContractState==3 && Partial.Week.Revenue==0 && Partial.RecentEntry(0).Kind==ECampusEntry::PartialContract,"partial club paid full contract");
        FCampusOperations Build; Build.Cash-=100; Build.RecordPathInvestment(100,480);
        Check(Build.PurchaseGym(480) && Build.Cash==39900 && Build.Investment==60100 && Build.Week.Expense==0,"paths before gym block purchase or mix expenses");
        std::cout << "PASS: grid paths, L quotes, atomic purchase, removal, BFS, alternate routes, connectivity, late arrival, anticipation, snapshots, group location, batch time, partial contract, investment\n";
        return 0;
    }
    catch(const std::exception& E) { std::cerr << "FAIL: " << E.what() << '\n'; return 1; }
}
