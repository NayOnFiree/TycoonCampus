#pragma once
#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"

namespace CampusUI
{
    inline FLinearColor Color(const TCHAR* Hex) { return FLinearColor::FromSRGBColor(FColor::FromHex(Hex)); }
    inline const FLinearColor Navy=Color(TEXT("10283F")), White=Color(TEXT("F8F9F7")), Ink=Color(TEXT("142940")), Muted=Color(TEXT("738397")), Mint=Color(TEXT("A7EDD0")), Green=Color(TEXT("185B58"));
    struct FTheme
    {
        FSlateRoundedBoxBrush Card{White,10.f}, Dock{Navy,12.f}, Track{Color(TEXT("DEE3E6")),5.f}, Fill{Color(TEXT("56B99A")),5.f};
        FSlateRoundedBoxBrush Hint{Color(TEXT("E3F3EA")),8.f};
        FButtonStyle Dark, Primary, Quiet, Speed;
        FProgressBarStyle Progress;
        FTheme()
        {
            Dark.SetNormal(FSlateRoundedBoxBrush(Navy,8.f)).SetHovered(FSlateRoundedBoxBrush(Color(TEXT("25415B")),8.f)).SetPressed(FSlateRoundedBoxBrush(Color(TEXT("346078")),8.f));
            Primary.SetNormal(FSlateRoundedBoxBrush(Green,8.f)).SetHovered(FSlateRoundedBoxBrush(Color(TEXT("247972")),8.f)).SetPressed(FSlateRoundedBoxBrush(Color(TEXT("123F3D")),8.f));
            Quiet.SetNormal(FSlateRoundedBoxBrush(FLinearColor::Transparent,6.f)).SetHovered(FSlateRoundedBoxBrush(Color(TEXT("E4EBEA")),6.f)).SetPressed(FSlateRoundedBoxBrush(Color(TEXT("D1E0DA")),6.f));
            Speed=Dark;
            Progress.SetBackgroundImage(Track).SetFillImage(Fill);
        }
    };
    inline const FTheme& Theme() { static FTheme T; return T; }
    enum class EIcon { Campus, Build, Road, Calendar, People, Finance, Settings, Close, Pause, Play };
    class SIcon : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SIcon) : _Kind(EIcon::Campus),_Tint(FLinearColor::White),_Size(24.f) {}
            SLATE_ARGUMENT(EIcon,Kind) SLATE_ATTRIBUTE(FLinearColor,Tint) SLATE_ARGUMENT(float,Size)
        SLATE_END_ARGS()
        void Construct(const FArguments& A) { Kind=A._Kind; Tint=A._Tint; Size=A._Size; }
        virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(Size,Size); }
        virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool Enabled) const override
        {
            const FVector2D Scale=G.GetLocalSize()/24.; const auto C=Tint.Get()*Style.GetColorAndOpacityTint();
            auto Line=[&](std::initializer_list<FVector2D> P)
            { TArray<FVector2D> Points; for(auto V:P) { Points.Add(V*Scale); }
                FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,C,true,1.8f); };
            switch(Kind)
            {
            case EIcon::Campus: Line({{3,8},{12,3},{21,8},{12,13},{3,8}}); Line({{3,12},{12,17},{21,12}}); Line({{3,16},{12,21},{21,16}}); break;
            case EIcon::Build: Line({{4,20},{15,9},{12,6},{15,3},{21,9},{18,12},{15,9}}); Line({{3,18},{6,21}}); break;
            case EIcon::Road: Line({{3,22},{8,2}}); Line({{21,22},{16,2}}); Line({{12,3},{12,7}}); Line({{12,11},{12,15}}); Line({{12,19},{12,22}}); break;
            case EIcon::Calendar: Line({{4,5},{20,5},{20,21},{4,21},{4,5}}); Line({{8,2},{8,7}}); Line({{16,2},{16,7}}); Line({{4,10},{20,10}}); Line({{8,14},{11,14},{11,17},{8,17},{8,14}}); break;
            case EIcon::People: Line({{9,3},{14,3},{16,6},{14,10},{9,10},{7,6},{9,3}}); Line({{3,22},{4,16},{8,13},{15,13},{19,16},{20,22},{3,22}}); Line({{19,4},{22,7},{20,10}}); break;
            case EIcon::Finance: Line({{4,21},{4,14},{7,14},{7,21}}); Line({{10,21},{10,8},{13,8},{13,21}}); Line({{16,21},{16,3},{19,3},{19,21}}); break;
            case EIcon::Settings: for(int I=0;I<8;++I) { double A=I*PI/4.; Line({{12+7*cos(A),12+7*sin(A)},{12+10*cos(A),12+10*sin(A)}}); } Line({{8,5},{16,5},{20,12},{16,19},{8,19},{4,12},{8,5}}); Line({{9,9},{15,9},{15,15},{9,15},{9,9}}); break;
            case EIcon::Close: Line({{6,6},{18,18}}); Line({{18,6},{6,18}}); break;
            case EIcon::Pause: Line({{8,4},{8,20}}); Line({{16,4},{16,20}}); break;
            case EIcon::Play: Line({{6,3},{20,12},{6,21},{6,3}}); break;
            }
            return Layer;
        }
    private:
        EIcon Kind; TAttribute<FLinearColor> Tint; float Size=24;
    };
}
