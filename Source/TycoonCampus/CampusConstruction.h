#pragma once
#include <cmath>
#include <cstdint>

struct FCampusFootprint
{
    int X = 0, Y = 0;
    bool Rotated = false;
    int Width() const { return Rotated ? 14 : 21; }
    int Height() const { return Rotated ? 21 : 14; }
    bool InsideParcel() const { return X >= 0 && Y >= 0 && X <= 100 - Width() && Y <= 100 - Height(); }
    double CenterX() const { return -10000.0 + (X + Width() * .5) * 200.0; }
    double CenterY() const { return -10000.0 + (Y + Height() * .5) * 200.0; }
    bool Overlaps(double MinX, double MinY, double MaxX, double MaxY) const
    {
        return CenterX() - Width() * 100 < MaxX && CenterX() + Width() * 100 > MinX
            && CenterY() - Height() * 100 < MaxY && CenterY() + Height() * 100 > MinY;
    }
    static FCampusFootprint AtCursor(double X, double Y, bool Rotated)
    {
        FCampusFootprint F; F.Rotated = Rotated;
        if (!std::isfinite(X) || !std::isfinite(Y) || std::abs(X) > 1e7 || std::abs(Y) > 1e7)
        { F.X = -1000; F.Y = -1000; return F; }
        F.X = static_cast<int>(std::floor((X + 10000) / 200.0 - F.Width() * .5 + .5));
        F.Y = static_cast<int>(std::floor((Y + 10000) / 200.0 - F.Height() * .5 + .5));
        return F;
    }
};
