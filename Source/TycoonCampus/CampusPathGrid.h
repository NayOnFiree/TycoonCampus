#pragma once
#include <cstdint>
#include <cmath>

/** Four-neighbour paths on the same 2 m grid as buildings. 1 = initial path, 2 = purchased. */
class FCampusPathGrid
{
public:
    unsigned char Cells[10000]{};
    bool Blocked[10000]{};
    int Route[10000]{};
    int RouteCount = 0;
    static bool Valid(int X, int Y) { return X >= 0 && X < 100 && Y >= 0 && Y < 100; }
    static int Cell(double World)
    {
        if (!std::isfinite(World) || World < -10000 || World >= 10000) { return -1; }
        return static_cast<int>(std::floor((World + 10000) / 200));
    }
    static double Center(int Cell) { return -9900 + Cell * 200; }
    // A visible 4 m tile groups four legacy 2 m cells; saves and routes retain their coordinates.
    static constexpr int TileStride=2;
    int Segment[10000]{};
    int SegmentCount = 0;
    bool MakeSegment(int X0, int Y0, int X1, int Y1, bool VerticalFirst)
    {
        SegmentCount = 0;
        if (!Valid(X0,Y0) || !Valid(X1,Y1)) { return false; }
        int X=X0, Y=Y0; Segment[SegmentCount++]=Y*100+X;
        for(int Leg=0;Leg<2;++Leg)
        {
            const bool Vertical = (Leg==0) == VerticalFirst;
            while ((Vertical ? Y : X) != (Vertical ? Y1 : X1))
            {
                if(Vertical) { Y += Y < Y1 ? 1 : -1; } else { X += X < X1 ? 1 : -1; }
                Segment[SegmentCount++]=Y*100+X;
            }
        }
        return true;
    }
    bool MakeRectangle(int X0, int Y0, int X1, int Y1, int Stride=TileStride)
    {
        SegmentCount=0;
        if((Stride!=1 && Stride!=2) || !Valid(X0,Y0) || !Valid(X1,Y1)) { return false; }
        X0=(X0/Stride)*Stride; Y0=(Y0/Stride)*Stride;
        X1=(X1/Stride)*Stride; Y1=(Y1/Stride)*Stride;
        const int MinX=X0<X1?X0:X1, MaxX=(X0>X1?X0:X1)+Stride-1;
        const int MinY=Y0<Y1?Y0:Y1, MaxY=(Y0>Y1?Y0:Y1)+Stride-1;
        for(int Y=MinY;Y<=MaxY;++Y) for(int X=MinX;X<=MaxX;++X)
        { Segment[SegmentCount++]=Y*100+X; }
        return true;
    }
    // -1 blocked/invalid, -2 protected initial path. Otherwise price (or 0 for removal).
    int Quote(bool Erase) const
    {
        if (!SegmentCount) { return -1; }
        int Price=0;
        for(int I=0;I<SegmentCount;++I)
        {
            const int C=Segment[I];
            if (Erase && Cells[C]==1) { return -2; }
            if (!Erase && Blocked[C]) { return -1; }
            if (!Erase && Cells[C]==0) { Price+=20; }
        }
        return Price;
    }
    bool Apply(bool Erase, std::int64_t& Cash)
    {
        const int Price=Quote(Erase);
        if(Price<0 || (!Erase && Cash<Price)) { return false; }
        Cash-=Price;
        for(int I=0;I<SegmentCount;++I) { const int C=Segment[I]; if(Erase) { Cells[C]=0; } else if(!Cells[C]) { Cells[C]=2; } }
        return true;
    }
    bool FindRoute(int Source, int Target)
    {
        RouteCount=0;
        if(Source<0 || Source>=10000 || Target<0 || Target>=10000 || !Cells[Source] || !Cells[Target]) { return false; }
        for(int I=0;I<10000;++I) { Parent[I]=-1; }
        int Head=0,Tail=0; Queue[Tail++]=Source; Parent[Source]=Source;
        while(Head<Tail && Parent[Target]<0)
        {
            const int C=Queue[Head++], X=C%100, Y=C/100;
            const int DX[]={1,-1,0,0}, DY[]={0,0,1,-1};
            for(int I=0;I<4;++I)
            {
                const int NX=X+DX[I], NY=Y+DY[I]; if(!Valid(NX,NY)) { continue; }
                const int N=NY*100+NX;
                if(Cells[N] && !Blocked[N] && Parent[N]<0) { Parent[N]=C; Queue[Tail++]=N; }
            }
        }
        if(Parent[Target]<0 || Blocked[Source]) { return false; }
        int C=Target; while(C!=Source) { Route[RouteCount++]=C; C=Parent[C]; }
        Route[RouteCount++]=Source;
        for(int I=0;I<RouteCount/2;++I) { const int V=Route[I]; Route[I]=Route[RouteCount-1-I]; Route[RouteCount-1-I]=V; }
        return true;
    }
private:
    int Parent[10000]{}, Queue[10000]{};
};
