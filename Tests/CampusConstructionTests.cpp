#include "../Source/TycoonCampus/CampusConstruction.h"
#include "../Source/TycoonCampus/CampusOperations.h"
#include <iostream>
#include <stdexcept>
#include <limits>
static void Check(bool V, const char* M) { if (!V) { throw std::runtime_error(M); } }
int main()
{
    try
    {
        auto F = FCampusFootprint::AtCursor(0, 0, false);
        Check(F.InsideParcel() && F.Width()==21 && F.Height()==14, "normal footprint");
        Check(FCampusFootprint::AtCursor(F.CenterX(), F.CenterY(), false).X == F.X, "snap not stable");
        Check(!FCampusFootprint::AtCursor(-10000, 0, false).InsideParcel(), "left boundary accepted");
        Check(!FCampusFootprint::AtCursor(10000, 0, true).InsideParcel(), "right boundary accepted");
        F = {0,0,false}; Check(F.InsideParcel() && F.CenterX()==-7900 && F.CenterY()==-8600, "parcel origin");
        F = {79,86,false}; Check(F.InsideParcel(), "last legal cells");
        F.X++; Check(!F.InsideParcel(), "one cell overflow");
        F = {86,79,true}; Check(F.InsideParcel() && F.Width()==14 && F.Height()==21, "rotated bounds");
        F = {0,0,false};
        Check(!F.Overlaps(-5800,-10000,-5000,-9000), "touching treated as overlap");
        Check(F.Overlaps(-5801,-10000,-5000,-9000), "partial overlap missed");
        Check(!FCampusFootprint::AtCursor(std::numeric_limits<double>::infinity(),0,false).InsideParcel(), "infinite cursor");
        FCampusOperations O;
        Check(O.PurchaseGym(480) && O.Cash==40000 && O.Investment==60000, "construction price");
        Check(O.Week.Expense==0 && O.Week.Profit()==0 && O.RecentEntry(0).Kind==ECampusEntry::Construction, "investment mixed into operations");
        Check(!O.PurchaseGym(480) && O.Cash==40000 && O.EntryCount()==1, "double purchase");
        FCampusOperations Poor; Poor.Cash=59999;
        Check(!Poor.PurchaseGym(480) && Poor.Cash==59999 && Poor.Investment==0 && Poor.EntryCount()==0, "unaffordable purchase mutated state");
        FCampusOperations Exact; Exact.Cash=60000;
        Check(Exact.PurchaseGym(480) && Exact.Cash==0, "exact price rejected");
        FCampusOperations Late; FCampusWeeklySchedule S;
        Check(Late.PurchaseGym(3*1440), "late purchase"); Late.AdvanceTo(72,S);
        Check(Late.Cash==40000 && Late.Week.Expense==0, "retroactive charges on purchase");
        Late.AdvanceTo(96,S); Check(Late.Cash==39900, "next midnight charges missing");
        FCampusOperations MidHour; S.Add(0,8,2,ECampusActivity::FreeAccess);
        Check(MidHour.PurchaseGym(510), "mid-hour purchase"); MidHour.AdvanceTo(10,S);
        Check(MidHour.Week.Completed==0, "activity started retroactively");
        std::cout << "PASS: grid, rotation, parcel, obstacles, invalid coordinates, atomic purchase, investment, late construction\n";
        return 0;
    }
    catch(const std::exception& E) { std::cerr << "FAIL: " << E.what() << '\n'; return 1; }
}
