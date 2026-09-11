#pragma once
#include <algorithm>

// Input is the allotted Slate space, never physical viewport pixels or an OS DPI guess.
struct FCampusPlanningLayout
{
    float Width;
    float Height;
    bool Compact;
    static FCampusPlanningLayout Evaluate(float AllottedWidth, float AllottedHeight)
    {
        const float Width = std::max(0.f, std::min(1248.f, AllottedWidth - 32.f));
        const float Height = std::max(0.f, std::min(740.f, AllottedHeight - 32.f));
        return {Width, Height, Width < 1100.f};
    }
};
