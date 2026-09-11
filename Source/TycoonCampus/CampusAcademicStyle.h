#pragma once
#include "CampusUIStyle.h"
#include "Brushes/SlateColorBrush.h"

// Candidate university theme. The legacy menu/planning keep their current theme until approval.
namespace CampusAcademicV2
{
    using CampusUI::Color;
    using CampusUI::SIcon;
    using CampusUI::EIcon;
    inline const FLinearColor Brown=Color(TEXT("30231F")), Cream=Color(TEXT("FAF3E7")), Ink=Color(TEXT("FAF3E7"));
    inline const FLinearColor Muted=Color(TEXT("D8C6B1")), OnDarkMuted=Color(TEXT("D8C6B1"));
    inline const FLinearColor Burgundy=Color(TEXT("762E40")), Sand=Color(TEXT("ECD9BA")), Line=Color(TEXT("D9C7AF"));
    inline constexpr float Radius=6.f, PanelPadding=22.f;
    inline constexpr int BodySize=14, CaptionSize=11, TitleSize=25;
    struct FTheme
    {
        // Neutral tint brushes let runtime tinting retain the specified palette, without multiplying two colors.
        FSlateColorBrush Surface{FLinearColor::White};
        FSlateColorBrush Card{Brown}, Dock{Brown};
        FSlateColorBrush Track{Color(TEXT("695448"))}, Fill{Burgundy}, Hint{Color(TEXT("48372F"))};
        FButtonStyle Dark, Primary, Quiet;
        FProgressBarStyle Progress;
        FTheme()
        {
            Dark.SetNormal(FSlateColorBrush(Color(TEXT("49372E"))))
                .SetHovered(FSlateColorBrush(Color(TEXT("514037"))))
                .SetPressed(FSlateColorBrush(Color(TEXT("674C3E"))))
                .SetDisabled(FSlateColorBrush(Color(TEXT("53463E"))));
            Primary.SetNormal(FSlateColorBrush(Burgundy))
                .SetHovered(FSlateColorBrush(Color(TEXT("90394F"))))
                .SetPressed(FSlateColorBrush(Color(TEXT("56202F"))))
                .SetDisabled(FSlateColorBrush(Color(TEXT("887568"))));
            Quiet.SetNormal(FSlateColorBrush(Color(TEXT("49372E"))))
                .SetHovered(FSlateColorBrush(Color(TEXT("675044"))))
                .SetPressed(FSlateColorBrush(Color(TEXT("392822"))));
            Progress.SetBackgroundImage(Track).SetFillImage(Fill);
        }
    };
    inline const FTheme& Theme() { static FTheme Value; return Value; }
}
