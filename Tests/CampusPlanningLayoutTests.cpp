#include "../Source/TycoonCampus/CampusPlanningLayout.h"
#include <iostream>
#include <stdexcept>

static void Check(bool Value, const char* Message)
{
    if (!Value) { throw std::runtime_error(Message); }
}

int main()
{
    try
    {
        // Stress spaces, not an assertion about Windows/Unreal DPI composition.
        for (float Scale : {1.f, 1.25f, 1.5f})
        {
            const float Width = 1280.f / Scale, Height = 720.f / Scale;
            const auto Layout = FCampusPlanningLayout::Evaluate(Width, Height);
            Check(Layout.Width + 32.f <= Width && Layout.Height + 32.f <= Height,
                "panel must stay inside allotted space with margins");
            Check(Layout.Compact == (Scale != 1.f), "stress spaces choose expected presentation");
            Check(Layout.Height >= 448.f, "supported stress spaces retain usable vertical space");
            if (!Layout.Compact)
            {
                Check(Layout.Width - 24.f - 320.f >= 728.f,
                    "wide view must fit seven day columns and the editor");
            }
        }
        for (float Width : {0.f, 16.f, 32.f, 400.f, 1131.f, 1132.f, 1920.f, 3840.f})
        {
            const auto Layout = FCampusPlanningLayout::Evaluate(Width, 1080.f);
            Check(Layout.Width >= 0.f && Layout.Width <= Width, "no overflow or negative size");
            Check(Layout.Width <= 1248.f && Layout.Height <= 740.f, "desktop size is capped");
            if (!Layout.Compact)
                Check(Layout.Width - 24.f - 320.f >= 728.f, "breakpoint preserves week width");
        }
        Check(FCampusPlanningLayout::Evaluate(1131.f, 720.f).Compact, "below breakpoint");
        Check(!FCampusPlanningLayout::Evaluate(1132.f, 720.f).Compact, "at breakpoint");
        std::cout << "Planning layout bounds and breakpoint passed\n";
    }
    catch (const std::exception& Error) { std::cerr << Error.what() << '\n'; return 1; }
}
