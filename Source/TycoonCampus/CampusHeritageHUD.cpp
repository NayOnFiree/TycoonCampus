#include "CampusHUD.h"
#include "CampusBuilding.h"
#include "CampusCameraPawn.h"
#include "CampusClock.h"
#include "CampusUIStyle.h"
#include "EngineUtils.h"
#include "Engine/GameViewportClient.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Fonts/CompositeFont.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Engine/UserInterfaceSettings.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

namespace CampusHeritage
{
    using CampusUI::Color;
    using CampusUI::SIcon;
    using CampusUI::EIcon;
    const FLinearColor Surface=Color(TEXT("FAFAF7")), Paper=Color(TEXT("F1F0EC"));
    const FLinearColor Ink=Color(TEXT("303735")), Muted=Color(TEXT("66706C")), Line=Color(TEXT("DDE1DB"));
    const FLinearColor Accent=Color(TEXT("82664E")), OnAccent=Color(TEXT("FFF5E6"));
    template<class T> T* Find(UWorld* W) { for(TActorIterator<T> I(W);I;++I) { return *I; } return nullptr; }
    FText Txt(const TCHAR* S) { return FText::FromString(S); }
    FSlateFontInfo Font(float Pixels,bool Bold=false)
    {
        // Use the Windows font already installed on the target platform, without redistributing it.
        const FString File=FPlatformMisc::GetEnvironmentVariable(TEXT("WINDIR"))/TEXT("Fonts")/(Bold?TEXT("seguisb.ttf"):TEXT("segoeui.ttf"));
        if(IFileManager::Get().FileExists(*File))
        {
            static TSharedPtr<const FCompositeFont> Fonts[2];
            const int Index=Bold?1:0;
            if(!Fonts[Index].IsValid()) { Fonts[Index]=MakeShared<FCompositeFont>(FName(TEXT("Default")),File,EFontHinting::Default,EFontLoadingPolicy::LazyLoad); }
            return FSlateFontInfo(Fonts[Index],Pixels*.75f,FName(TEXT("Default")));
        }
        return FCoreStyle::GetDefaultFontStyle(Bold?"Bold":"Regular",Pixels*.75f);
    }
    TSharedRef<SWidget> Text(TAttribute<FText> Value,float Pixels=11,FLinearColor Tint=Ink,bool Bold=false)
    { return SNew(STextBlock).Text(Value).Font(Font(Pixels,Bold)).ColorAndOpacity(Tint).AutoWrapText(true); }
    struct FBox : FSlateBrush
    {
        FBox(const FString& File,const FMargin& Border,float Size=32)
            :FSlateBrush(ESlateBrushDrawType::Box,FName(*File),Border,ESlateBrushTileType::NoTile,ESlateBrushImageType::FullColor,FVector2D(Size,Size),FLinearColor::White,nullptr,true) {}
    };
    struct FTheme
    {
        FBox Panel{FPaths::ProjectContentDir()/TEXT("UI/Heritage/panel.png"),FMargin(.25f)};
        FBox Shadow{FPaths::ProjectContentDir()/TEXT("UI/Heritage/shadow.png"),FMargin(.25f),64};
        FSlateColorBrush PaperBrush{Paper},LineBrush{Line},AccentBrush{Accent};
        FBox Badge{FPaths::ProjectContentDir()/TEXT("UI/Heritage/status.png"),FMargin(.15625f)};
        FButtonStyle Primary,Secondary,Quiet,Speed,Selected;
        FProgressBarStyle Meter;
        FTheme()
        {
            auto Brush=[](const TCHAR* Name)
            {
                FSlateBrush B=*FCoreStyle::Get().GetBrush("WhiteBrush");
                const FString N(Name);
                B.TintColor=N==TEXT("primary")?Accent:N==TEXT("primary-hover")?Color(TEXT("92765E")):N==TEXT("pressed")?Color(TEXT("6F5540")):N.Contains(TEXT("hover"))?Line:N==TEXT("speed")?Paper:FLinearColor::White;
                return B;
            };
            Primary.SetNormal(Brush(TEXT("primary"))).SetHovered(Brush(TEXT("primary-hover"))).SetPressed(Brush(TEXT("pressed")));
            Secondary.SetNormal(Brush(TEXT("secondary"))).SetHovered(Brush(TEXT("secondary-hover"))).SetPressed(Brush(TEXT("speed")));
            Quiet.SetNormal(Brush(TEXT("quiet"))).SetHovered(Brush(TEXT("quiet-hover"))).SetPressed(Brush(TEXT("speed")));
            Speed.SetNormal(Brush(TEXT("speed"))).SetHovered(Brush(TEXT("secondary-hover"))).SetPressed(Brush(TEXT("speed")));
            Selected=Primary;
            Meter.SetBackgroundImage(LineBrush).SetFillImage(AccentBrush);
        }
    };
    const FTheme& WhitePanelStyle() { static FTheme S; return S; }
    TSharedRef<SWidget> Panel(TSharedRef<SWidget> Content,FMargin Padding=FMargin(0))
    {
        // Explicit opaque background for every management panel; no external texture.
        return SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(1.f,1.f,1.f,1.f))
            .ColorAndOpacity(FLinearColor::White)
            .Padding(Padding)[Content];
    }
    TSharedRef<SWidget> Meter(TAttribute<TOptional<float>> Value,float Height=4)
    { return SNew(SBox).HeightOverride(Height)[SNew(SProgressBar).Style(&WhitePanelStyle().Meter).BorderPadding(FVector2D::ZeroVector).Percent(Value).FillColorAndOpacity(FLinearColor::White)]; }
}

TSharedRef<SWidget> ACampusHUD::BuildHeritageHUD()
{
    using namespace CampusHeritage;
    auto Root=SNew(SOverlay).Visibility(EVisibility::SelfHitTestInvisible);
    auto Visible=TAttribute<EVisibility>::CreateLambda([this]() { return IsModalOpen()?EVisibility::Collapsed:EVisibility::Visible; });
    auto Money=[](int64 V) { FString S=FString::Printf(TEXT("%lld"),FMath::Abs(V)); for(int I=S.Len()-3;I>0;I-=3) { S.InsertAt(I,TEXT(' ')); } return (V<0?TEXT("-"):TEXT(""))+S+TEXT(" €"); };
    auto Action=[this](int I) { if(auto* P=Cast<ACampusCameraPawn>(GetOwningPawn())) { P->InterfaceAction(I); } };
    auto TypeName=[this]()
    {
        switch (GetConstructionType())
        {
        case 0: return FText::FromString(TEXT("Gymnase"));
        case 1: return FText::FromString(TEXT("Terrain de foot"));
        case 2: return FText::FromString(TEXT("Terrain de tennis"));
        default: return FText::FromString(TEXT("Décoration"));
        }
    };
    auto TypeIsEnabled=[this](int32 Index)
    {
        return Index >= 0 && Index <= 3;
    };
    auto TypeIndexLabel=[this](int32 Index)
    {
        switch (Index)
        {
        case 0: return FText::FromString(TEXT("Gymnase"));
        case 1: return FText::FromString(TEXT("Terrain de foot"));
        case 2: return FText::FromString(TEXT("Terrain de tennis"));
        default: return FText::FromString(TEXT("Décoration"));
        }
    };
    auto Button=[](const TCHAR* Label,TFunction<void()> Click,bool Primary=false)
    {
        return SNew(SButton).ButtonStyle(Primary?&WhitePanelStyle().Primary:&WhitePanelStyle().Secondary).IsFocusable(false).ContentPadding(FMargin(12,10)).HAlign(HAlign_Center)
            .OnClicked_Lambda([Click]() { Click(); return FReply::Handled(); })[Text(Txt(Label),11,Primary?OnAccent:Ink,true)];
    };
    auto Top=SNew(SHorizontalBox);
    Top->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,18,0)
        [SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SBox).WidthOverride(23).HeightOverride(27)[SNew(SBorder).BorderImage(&WhitePanelStyle().Panel).Padding(0).HAlign(HAlign_Center).VAlign(VAlign_Center)[Text(Txt(TEXT("C")),16)]]]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0,0,0)[Text(Txt(TEXT("C A M P U S")),12,Ink,true)]];
    auto Resource=[&](const TCHAR* Caption,TAttribute<FText> Value,FLinearColor Tint=Ink)
    {
        Top->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,18,0)
        [SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(1).HeightOverride(25)[SNew(SImage).Image(&WhitePanelStyle().LineBrush)]]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16,0,8,0)[SNew(SBox).WidthOverride(55)[Text(Txt(Caption),8,Muted,true)]]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[Text(Value,15,Tint,true)]];
    };
    Resource(TEXT("BUDGET\nDISPONIBLE"),TAttribute<FText>::CreateLambda([this,Money]() { auto* B=Find<ACampusBuilding>(GetWorld()); return FText::FromString(B?Money(B->GetOperations().Cash):TEXT("—")); }));
    Resource(TEXT("CETTE\nSEMAINE"),TAttribute<FText>::CreateLambda([this,Money]() { auto* B=Find<ACampusBuilding>(GetWorld()); if(!B) { return Txt(TEXT("—")); } auto V=B->GetOperations().Week.Profit(); return FText::FromString((V>=0?TEXT("+"):TEXT(""))+Money(V)); }),Accent);
    Top->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,16,0)[Text(TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return B?FText::FromString(FString::Printf(TEXT("%d réservations   ·   %d personnel"),B->GetSchedule().GetReservationCount(),B->GetSchedule().IsStaffHired()?1:0)):FText::GetEmpty(); }),10,Muted)];
    Top->AddSlot().AutoWidth().VAlign(VAlign_Center)[SNew(SButton).ButtonStyle(&WhitePanelStyle().Quiet).ContentPadding(4).IsFocusable(false).ToolTipText(Txt(TEXT("Menu [Échap]"))).OnClicked_Lambda([this]() { ToggleMenu(); return FReply::Handled(); })[SNew(SIcon).Kind(EIcon::Settings).Size(16).Tint(Muted)]];
    Root->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(16,16,16,0)
    [SNew(SBox).HeightOverride(48).Visibility(Visible)[Panel(Top,FMargin(16,0))]];

    auto Dock=SNew(SHorizontalBox);
    const TCHAR* Names[]={TEXT("Construire"),TEXT("Chemins"),TEXT("Planning"),TEXT("Personnel"),TEXT("Finances")};
    const TCHAR* Tips[]={TEXT("Construire [B]"),TEXT("Chemins [C]"),TEXT("Planning [P]"),TEXT("Personnel [E]"),TEXT("Finances [F]")};
    const EIcon Icons[]={EIcon::Build,EIcon::Road,EIcon::Calendar,EIcon::People,EIcon::Finance};
    for(int I=0;I<5;++I)
    {
        auto Active=[this,I]() { auto* P=Cast<ACampusCameraPawn>(GetOwningPawn()); return P && ((I==0&&P->IsConstructing())||(I==1&&P->IsPathMode())); };
        auto Tile=SNew(SButton).ButtonStyle(&WhitePanelStyle().Quiet).IsFocusable(false).ContentPadding(FMargin(10,8)).ToolTipText(Txt(Tips[I]))
            .OnClicked_Lambda([this,Action,I]() { if(I<2) { Action(I); } else { OpenPage(I-2); } return FReply::Handled(); });
        // Overlay the selected treatment while retaining native button hover/pressed handling.
        auto Contents=SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SIcon).Kind(Icons[I]).Size(17).Tint_Lambda([Active]() { return Active()?OnAccent:Muted; })]
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(7,0,0,0)[SNew(STextBlock).Text(Txt(Names[I])).Font(Font(10)).ColorAndOpacity_Lambda([Active]() { return FSlateColor(Active()?OnAccent:Muted); })];
        Tile->SetContent(Contents);
        auto Selected=SNew(SButton).ButtonStyle(&WhitePanelStyle().Selected).IsFocusable(false).ContentPadding(FMargin(10,8)).ToolTipText(Txt(Tips[I]))
            .Visibility_Lambda([Active]() { return Active()?EVisibility::Visible:EVisibility::Collapsed; })
            .OnClicked_Lambda([Action,I]() { Action(I); return FReply::Handled(); })
            [SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SIcon).Kind(Icons[I]).Size(17).Tint(OnAccent)]
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(7,0,0,0)[Text(Txt(Names[I]),10,OnAccent)]];
        Tile->SetVisibility(TAttribute<EVisibility>::CreateLambda([Active]() { return Active()?EVisibility::Collapsed:EVisibility::Visible; }));
        Dock->AddSlot().AutoWidth().Padding(0,0,I==4?0:2,0)[SNew(SBox).MinDesiredWidth(66)[SNew(SOverlay)+SOverlay::Slot()[Tile]+SOverlay::Slot()[Selected]]];
    }
    Root->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(16,0,0,16)[SNew(SBox).Visibility(Visible)[Panel(Dock,FMargin(4))]];

    auto TypeBar=SNew(SHorizontalBox);
    for(int I=0;I<4;++I)
    {
        auto IsActive=[this,I]() { return GetConstructionType() == I; };
        auto IsEnabled=TypeIsEnabled(I);
        auto ButtonColor=[IsEnabled,IsActive]() { return IsEnabled ? (IsActive() ? Muted : Ink) : FLinearColor(0.68f, 0.70f, 0.68f); };
        auto TypeButton=SNew(SButton)
            .ButtonStyle(IsActive() ? &WhitePanelStyle().Selected : (IsEnabled ? &WhitePanelStyle().Secondary : &WhitePanelStyle().Quiet))
            .IsFocusable(false).ContentPadding(FMargin(8,6))
            .IsEnabled_Lambda([this,I,TypeIsEnabled]() { return TypeIsEnabled(I); })
            .ToolTipText_Lambda([this,I,TypeIndexLabel]() { return TypeIndexLabel(I); })
            .OnClicked_Lambda([this,I,TypeIsEnabled]() { if(TypeIsEnabled(I)) { SetConstructionType(I); } return FReply::Handled(); })
            [Text(TAttribute<FText>::CreateLambda([this,I,TypeIndexLabel]() { return TypeIndexLabel(I); }),10,ButtonColor(),true)];
        TypeBar->AddSlot().AutoWidth().Padding(0,0,I==3?0:4,0)[TypeButton];
    }
    Root->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(16,0,0,82)[SNew(SBox).Visibility(Visible)[Panel(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[Text(FText::FromString(TEXT("Type de construction")),9,Muted)]
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,4)[TypeBar]
        + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[Text(TAttribute<FText>::CreateLambda([this]() { return FText::FromString(FString::Printf(TEXT("Type actif : %s"), *GetConstructionTypeName().ToString())); }),9,Accent)]]);

    auto ClockBox=SNew(SVerticalBox);
    ClockBox->AddSlot().AutoHeight().Padding(12,8,12,0)[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().FillWidth(1)[Text(TAttribute<FText>::CreateLambda([this]() { auto* C=Find<ACampusClock>(GetWorld()); auto M=C?C->GetTotalMinutes():0; return FText::FromString(FString::Printf(TEXT("SEMESTRE %lld · SEM. %02lld"),M/40320+1,M/10080%4+1)); }),8,Muted)]
        +SHorizontalBox::Slot().AutoWidth()[Text(TAttribute<FText>::CreateLambda([this]() { auto* C=Find<ACampusClock>(GetWorld()); return Txt(C&&C->IsSimulationPaused()?TEXT("EN PAUSE"):TEXT("EN COURS")); }),8,Accent,true)]];
    auto SpeedBar=SNew(SHorizontalBox);
    for(int I=0;I<4;++I)
    {
        const int Speed=I==0?0:1<<(I-1);
        auto Active=[this,Speed]() { auto* C=Find<ACampusClock>(GetWorld()); return C&&(Speed==0?C->IsSimulationPaused():!C->IsSimulationPaused()&&C->GetSimulationSpeed()==Speed); };
        auto Click=[this,Speed]() { if(auto* C=Find<ACampusClock>(GetWorld())) { if(Speed==0) { C->TogglePause(); } else { C->SetSimulationSpeed(Speed); if(C->IsSimulationPaused()) { C->TogglePause(); } } } return FReply::Handled(); };
        auto Pair=SNew(SOverlay);
        for(int State=0;State<2;++State)
        {
            Pair->AddSlot()[SNew(SButton).ButtonStyle(State?&WhitePanelStyle().Selected:&WhitePanelStyle().Speed).ContentPadding(0).IsFocusable(false).HAlign(HAlign_Center).VAlign(VAlign_Center)
                .Visibility_Lambda([Active,State]() { return Active()==(State!=0)?EVisibility::Visible:EVisibility::Collapsed; })
                .ToolTipText(Txt(I==0?TEXT("Pause / reprendre [Espace]"):I==1?TEXT("Vitesse ×1"):I==2?TEXT("Vitesse ×2"):TEXT("Vitesse ×4")))
                .OnClicked_Lambda(Click)[Text(FText::FromString(I==0?TEXT("Ⅱ"):FString::Printf(TEXT("×%d"),Speed)),10,State?OnAccent:Ink)]];
        }
        SpeedBar->AddSlot().AutoWidth().Padding(I?3:0,0,0,0)[SNew(SBox).WidthOverride(28).HeightOverride(28)[Pair]];
    }
    ClockBox->AddSlot().AutoHeight().Padding(10,6,10,9)[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this]() { auto* C=Find<ACampusClock>(GetWorld()); auto M=C?C->GetTotalMinutes():0; return FText::FromString(FString::Printf(TEXT("%02lld:%02lld"),M/60%24,M%60)); }),20,Ink,true)]
            +SVerticalBox::Slot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this]() { const TCHAR* Days[]={TEXT("Lundi"),TEXT("Mardi"),TEXT("Mercredi"),TEXT("Jeudi"),TEXT("Vendredi"),TEXT("Samedi"),TEXT("Dimanche")}; auto* C=Find<ACampusClock>(GetWorld()); return Txt(Days[C?C->GetTotalMinutes()/1440%7:0]); }),9,Muted)]]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10,0,0,0)[SpeedBar]];
    ClockBox->AddSlot().AutoHeight()[Meter(TAttribute<TOptional<float>>::CreateLambda([this]() { auto* C=Find<ACampusClock>(GetWorld()); return TOptional<float>(C?(C->GetTotalMinutes()%1440)/1440.f:0.f); }),2)];
    Root->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0,0,16,16)[SNew(SBox).WidthOverride(262).Visibility(Visible)[Panel(ClockBox,FMargin(1))]];

    // Inspector-only repair. Do not change the still-unvalidated surrounding HUD.
    {
    const FSlateBrush* Solid=FCoreStyle::Get().GetBrush("WhiteBrush");
    auto Text=[](TAttribute<FText> Value,float Pixels=11,FLinearColor Tint=Ink,bool Bold=false,bool Wrap=false)->TSharedRef<SWidget>
    {
        return SNew(STextBlock).Text(Value).Font(Font(Pixels,Bold)).ColorAndOpacity(Tint)
            .AutoWrapText(Wrap); // Numeric AutoWidth slots must report their full, unwrapped desired size.
    };
    auto Panel=[Solid](TSharedRef<SWidget> Content,FMargin Padding=FMargin(0))->TSharedRef<SWidget>
    {
        return SNew(SBorder).BorderImage(Solid).BorderBackgroundColor(Line).Padding(1)
            [SNew(SBorder).BorderImage(Solid).BorderBackgroundColor(FLinearColor::White).Padding(Padding)[Content]];
    };
    auto GymButton=[Solid](const TCHAR* Label,TFunction<void()> Click,bool Primary=false)
    {
        // Fresh styles are owned by these buttons, not by a process-lifetime Live Coding singleton.
        auto Owned=MakeShared<FButtonStyle>();
        FSlateBrush Normal=*Solid,Hover=*Solid,Pressed=*Solid;
        Normal.TintColor=Primary?Accent:Paper;
        Hover.TintColor=Primary?Color(TEXT("92765E")):Color(TEXT("E7E8E2"));
        Pressed.TintColor=Primary?Color(TEXT("6F5540")):Line;
        Owned->SetNormal(Normal).SetHovered(Hover).SetPressed(Pressed);
        return SNew(SButton).ButtonStyle(&Owned.Get()).IsFocusable(false).ContentPadding(FCString::Strcmp(Label,TEXT("×"))==0?FMargin(0):FMargin(12,10)).HAlign(HAlign_Center)
            .OnClicked_Lambda([Click,Owned]() { Click(); return FReply::Handled(); })
            [SNew(STextBlock).Text(Txt(Label)).Font(Font(11,true)).ColorAndOpacity(Primary?OnAccent:Ink).AutoWrapText(false)];
    };
    auto Meter=[Solid](TAttribute<TOptional<float>> Value,float Height=4)->TSharedRef<SWidget>
    {
        auto Owned=MakeShared<FProgressBarStyle>();
        FSlateBrush Track=*Solid,Fill=*Solid; Track.TintColor=Line; Fill.TintColor=Accent;
        Owned->SetBackgroundImage(Track).SetFillImage(Fill);
        return SNew(SBox).HeightOverride(Height)[SNew(SProgressBar).Style(&Owned.Get()).BorderPadding(FVector2D::ZeroVector).FillColorAndOpacity(FLinearColor::White)
            .Percent_Lambda([Value,Owned]() { return Value.Get(); })];
    };
    UE_LOG(LogTemp,Display,TEXT("GymInspector repair v1: legacy brush draw=%d alpha=%.2f dynamic=%d file=%d; native draw=%d alpha=%.2f"),
        int32(WhitePanelStyle().Panel.DrawAs),WhitePanelStyle().Panel.GetTint(FWidgetStyle()).A,WhitePanelStyle().Panel.IsDynamicallyLoaded(),IFileManager::Get().FileExists(*WhitePanelStyle().Panel.GetResourceName().ToString()),int32(Solid->DrawAs),Solid->GetTint(FWidgetStyle()).A);
    auto Body=SNew(SVerticalBox);
    Body->AddSlot().AutoHeight().Padding(0,0,0,10)[Text(Txt(TEXT("Gymnase 01")),19,Ink,true)];
    Body->AddSlot().AutoHeight().HAlign(HAlign_Left)[SNew(SBorder).BorderImage(Solid).BorderBackgroundColor(Color(TEXT("E4ECDC"))).Padding(6,4)[Text(TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); auto* C=Find<ACampusClock>(GetWorld()); if(!B) { return FText::GetEmpty(); } if(!B->IsConnected()) { return Txt(TEXT("● Non raccordé")); } if(B->GetOperations().IsActive()) { return Txt(B->GetOperations().ActiveCanRun()?TEXT("● Séance en cours"):TEXT("● Séance empêchée")); } int H=C?C->GetTotalMinutes()/60%24:8; return Txt(H<8||H>=22?TEXT("● Fermé"):TEXT("● Ouvert · Disponible")); }),10,Color(TEXT("3C5D36")))]];
    auto Row=[&](const TCHAR* Name,TAttribute<FText> Value)
    {
        Body->AddSlot().AutoHeight().Padding(0,11)[SNew(SHorizontalBox)+SHorizontalBox::Slot().FillWidth(1)[Text(Txt(Name),11,Muted)]+SHorizontalBox::Slot().AutoWidth().Padding(14,0,0,0)[Text(Value,11,Ink,true)]];
        Body->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(1)[SNew(SImage).Image(Solid).ColorAndOpacity(Line)]];
    };
    Row(TEXT("Horaires d'ouverture"),Txt(TEXT("08:00 — 22:00")));
    Row(TEXT("Participants actuels"),TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return FText::AsNumber(B?B->GetOperations().ActiveParticipants():0); }));
    Body->AddSlot().AutoHeight().Padding(0,14,0,8)[Text(Txt(TEXT("OCCUPATION PLANIFIÉE")),9,Muted,true)];
    Body->AddSlot().AutoHeight()[SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth()[Text(TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return FText::FromString(FString::Printf(TEXT("%.0f %%"),B?100.*B->GetSchedule().GetBookedHours()/98.:0.)); }),22,Ink,true)]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom).Padding(8,0,0,2)[Text(Txt(TEXT("cette semaine")),11,Muted)]];
    Body->AddSlot().AutoHeight().Padding(0,9,0,7)[Meter(TAttribute<TOptional<float>>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return TOptional<float>(B?B->GetSchedule().GetBookedHours()/98.f:0.f); }))];
    Body->AddSlot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return FText::FromString(FString::Printf(TEXT("%d heures réservées sur 98 disponibles"),B?B->GetSchedule().GetBookedHours():0)); }),11,Muted,false,true)];
    Body->AddSlot().AutoHeight().Padding(0,12,0,0)[GymButton(TEXT("Ouvrir le planning"),[this]() { OpenPage(0); },true)];
    Body->AddSlot().AutoHeight().Padding(0,6,0,0)[GymButton(TEXT("Consulter les finances"),[this]() { OpenPage(2); })];
    Body->AddSlot().AutoHeight().Padding(0,6,0,0)[SNew(SBox).Visibility_Lambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return B&&(!B->IsConnected()||B->GetSchedule().GetUnstaffedCount()>0||(B->GetOperations().ContractState==1&&!B->GetSchedule().HasContractBooking()))?EVisibility::Visible:EVisibility::Collapsed; })[GymButton(TEXT("Corriger les alertes"),[this]() { OpenAlert(); })]];
    Body->AddSlot().AutoHeight().Padding(0,10,0,0)[SNew(SExpandableArea).InitiallyCollapsed(true).BorderImage(Solid).BodyBorderImage(Solid).BorderBackgroundColor(Surface).BodyBorderBackgroundColor(Surface)
        .HeaderContent()[Text(Txt(TEXT("Activité et suivi")),10,Muted)]
        .BodyContent()[Text(TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); if(!B) { return FText::GetEmpty(); } const auto& O=B->GetOperations(); return FText::FromString(FString::Printf(TEXT("%s\nTrajet : %.0f m · %d figurants\nPréparation : %d h\nUniversité : %d / 360 min\nÉquipe : %d / 240 min"),*B->GetActivityStatus(),B->GetRouteMetres(),B->GetVisiblePeople(),B->GetSchedule().GetPreparationHours(),O.Week.UniversityMinutes,O.Week.TrainingMinutes)); }),11,Muted,false,true)]];
    Body->AddSlot().AutoHeight().Padding(0,6,0,0)[Text(TAttribute<FText>::CreateLambda([this]()
    {
        auto* B=Find<ACampusBuilding>(GetWorld()); auto* C=Find<ACampusClock>(GetWorld());
        if(!B||!C) { return FText::GetEmpty(); }
        const int64 H=C->GetTotalMinutes()/60;
        for(int64 N=H+1;N<=H+168;++N)
        { const auto* R=B->GetSchedule().FindAt(N/24%7,N%24); if(R&&R->StartHour==N%24) { return FText::FromString(FString::Printf(TEXT("Prochaine séance dans %lld h"),N-H)); } }
        return Txt(TEXT("Aucune séance programmée"));
    }),10,Muted)];
    auto Inspector=SNew(SVerticalBox);
    Inspector->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(46)[SNew(SBorder).BorderImage(Solid).BorderBackgroundColor(Paper).Padding(FMargin(14,10))
        [SNew(SHorizontalBox)+SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[Text(Txt(TEXT("INSTALLATIONS SPORTIVES")),9,Muted,true)]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SBox).WidthOverride(28).HeightOverride(26)[GymButton(TEXT("×"),[this]() { if(auto* B=Find<ACampusBuilding>(GetWorld())) { B->SetSelected(false); } })]]]]];
    Inspector->AddSlot().AutoHeight().Padding(15)[Body];
    Root->AddSlot().HAlign(HAlign_Right).VAlign(VAlign_Top)
        [SNew(SDPIScaler).DPIScale_Lambda([this]()
        {
            FVector2D V; GetWorld()->GetGameViewport()->GetViewportSize(V);
            const float Scale=GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(FMath::RoundToInt(V.X),FMath::RoundToInt(V.Y)));
            return 1.f/FMath::Max(Scale,.1f);
        })
        .Visibility_Lambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); auto* P=Cast<ACampusCameraPawn>(GetOwningPawn()); return !IsModalOpen()&&B&&B->IsSelected()&&P&&!P->IsConstructing()&&!P->IsPathMode()?EVisibility::Visible:EVisibility::Collapsed; })
        [SNew(SBox).Padding(FMargin(0,80,16,100))
        [SNew(SBox).WidthOverride(252).MaxDesiredHeight_Lambda([this]() { FVector2D V; GetWorld()->GetGameViewport()->GetViewportSize(V); return FMath::Max(160.f,float(V.Y)-185.f); })

        [Panel(SNew(SScrollBox)+SScrollBox::Slot()[Inspector])]]]];
    }

    Root->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(18,80,0,0)
        [SNew(SBox).Visibility(Visible)[SNew(SVerticalBox)
        +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(Txt(TEXT("Tycoon Campus"))).Font(Font(11)).ColorAndOpacity(Ink).ShadowOffset(FVector2D(0,1)).ShadowColorAndOpacity(Surface)]
        +SVerticalBox::Slot().AutoHeight().Padding(0,5,0,0)[SNew(STextBlock).Text(Txt(TEXT("CAMPUS SPORTIF"))).Font(Font(9)).ColorAndOpacity(Ink).ShadowOffset(FVector2D(0,1)).ShadowColorAndOpacity(Surface)]]];
    auto Steps=[this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return B?(int(B->IsBuilt())+int(B->IsConnected())+int(B->GetSchedule().GetReservationCount()>0)):0; };
    auto Objective=SNew(SVerticalBox);
    Objective->AddSlot().AutoHeight()[Text(Txt(TEXT("VOTRE PROCHAIN OBJECTIF")),9,Muted,true)];
    Objective->AddSlot().AutoHeight().Padding(0,7)[Text(Txt(TEXT("Un campus qui prend vie")),12,Ink,true)];
    Objective->AddSlot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([this]() { auto* B=Find<ACampusBuilding>(GetWorld()); return Txt(!B||!B->IsBuilt()?TEXT("Construisez votre premier gymnase."):!B->IsConnected()?TEXT("Reliez le gymnase aux chemins du campus."):B->GetSchedule().GetReservationCount()==0?TEXT("Programmez votre première séance."):TEXT("Votre campus est prêt. Développez son planning.")); }),10,Muted)];
    Objective->AddSlot().AutoHeight().Padding(0,9,0,7)[Meter(TAttribute<TOptional<float>>::CreateLambda([Steps]() { return TOptional<float>(Steps()/3.f); }))];
    Objective->AddSlot().AutoHeight()[Text(TAttribute<FText>::CreateLambda([Steps]() { return FText::FromString(FString::Printf(TEXT("%d / 3 étapes accomplies"),Steps())); }),11,Muted)];
    Root->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(18,128,0,0)[SNew(SBox).WidthOverride(215).Visibility(Visible)[Panel(SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(2)[SNew(SImage).Image(&WhitePanelStyle().AccentBrush)]]+SHorizontalBox::Slot().FillWidth(1).Padding(12,12,14,12)[Objective],FMargin(1))]];
    Root->AddSlot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(18,0,300,72)[SNew(SBox).WidthOverride(300)
        .Visibility_Lambda([this]() { auto* P=Cast<ACampusCameraPawn>(GetOwningPawn()); return !IsModalOpen()&&P&&(P->IsConstructing()||P->IsPathMode()||P->SaveStatus!=TEXT("F5 : sauvegarder / F9 : charger / F10 : copie de secours"))?EVisibility::Visible:EVisibility::Collapsed; })
        [Panel(Text(TAttribute<FText>::CreateLambda([this]() { auto* P=Cast<ACampusCameraPawn>(GetOwningPawn()); return !P?FText::GetEmpty():FText::FromString(P->IsConstructing()?P->GetConstructionStatus():P->IsPathMode()?TEXT("Glisser : rectangle · X : supprimer · C : quitter\n")+P->GetPathStatus():P->SaveStatus); }),11,Ink),FMargin(14,12))]];
    return Root;
}
