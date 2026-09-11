#include "CampusHUD.h"
#include "CampusUIStyle.h"
#include "CampusAcademicStyle.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "CampusClock.h"
#include "CampusBuilding.h"
#include "CampusCameraPawn.h"
#include "SCampusPlanningPanel.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "Widgets/SViewport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace
{
    class FCampusMenuInput : public IInputProcessor
    {
    public:
        TWeakObjectPtr<ACampusHUD> HUD;
        TFunction<bool()> HasInterfaceFocus;
        explicit FCampusMenuInput(ACampusHUD* InHUD) : HUD(InHUD) {}
        virtual void Tick(float,FSlateApplication&,TSharedRef<ICursor>) override {}
        virtual bool HandleKeyDownEvent(FSlateApplication&,const FKeyEvent& Event) override
        {
            if(Event.GetKey()!=EKeys::Escape || !HUD.IsValid() || !HUD->GetWorld()->GetGameViewport()) { return false; }
            auto V=HUD->GetWorld()->GetGameViewport()->GetGameViewportWidget();
            if((!V.IsValid() || (!V->HasKeyboardFocus() && !V->HasFocusedDescendants()))
                && (!HasInterfaceFocus || !HasInterfaceFocus())) { return false; }
            if(!Event.IsRepeat()) { HUD->ToggleMenu(); }
            return true;
        }
    };
    const FLinearColor Panel=CampusUI::Navy;
    template<class T> T* Actor(UWorld* W) { for(TActorIterator<T> It(W);It;++It) { return *It; } return nullptr; }
    class SCampusInterface : public SCompoundWidget
    {
    public:
        SLATE_BEGIN_ARGS(SCampusInterface) {} SLATE_ARGUMENT(TWeakObjectPtr<ACampusHUD>, HUD) SLATE_END_ARGS()
        void Construct(const FArguments& Args) { HUD=Args._HUD; }
        void SetContent(TSharedRef<SWidget> Content) { ChildSlot[Content]; }
        virtual bool SupportsKeyboardFocus() const override { return true; }
        // Only receives pointer events routed through a visible UI child; the terrain remains hit-test transparent.
        virtual FReply OnMouseButtonDown(const FGeometry&,const FPointerEvent&) override { return FReply::Handled(); }
        virtual FReply OnMouseWheel(const FGeometry&,const FPointerEvent&) override { return FReply::Handled(); }
        virtual FReply OnPreviewKeyDown(const FGeometry&,const FKeyEvent& E) override
        {
            if(!HUD.IsValid()) { return FReply::Unhandled(); }
            if(E.GetKey()==EKeys::Escape) { if(!E.IsRepeat()) { HUD->ToggleMenu(); } return FReply::Handled(); }
            if(HUD->IsMenuOpen())
            {
                if(!E.IsRepeat()) { if(auto* A=Cast<ACampusCameraPawn>(HUD->GetOwningPawn()))
                { if(E.GetKey()==EKeys::F5) { A->SaveCampus(); } else if(E.GetKey()==EKeys::F9) { A->LoadCampus(); } else if(E.GetKey()==EKeys::F10) { A->LoadBackup(); } } }
                return FReply::Handled();
            }
            return FReply::Unhandled();
        }
    private:
        TWeakObjectPtr<ACampusHUD> HUD;
    };
    TSharedRef<SWidget> Label(TAttribute<FText> Text,int Size=11)
    { return SNew(STextBlock).Text(Text).ColorAndOpacity(CampusUI::White).AutoWrapText(true).Font(FCoreStyle::GetDefaultFontStyle("Regular",Size)); }
}

void ACampusHUD::BeginPlay() { Super::BeginPlay(); BuildInterface(); }
void ACampusHUD::DrawHUD() { Super::DrawHUD(); }

void ACampusHUD::BuildInterface()
{
    auto* Viewport=GetWorld()->GetGameViewport(); if(!Viewport) { return; }
    auto Text=[](const TCHAR* T) { return FText::FromString(T); };
    auto Button=[](const TCHAR* Title,TFunction<void()> Action)
    {
        return SNew(SButton).ButtonStyle(&CampusUI::Theme().Dark).IsFocusable(false).ContentPadding(FMargin(14,10))
            .OnClicked_Lambda([Action]() { Action(); return FReply::Handled(); })[Label(FText::FromString(Title),12)];
    };
    auto Status=TAttribute<FText>::CreateLambda([this]() { auto* A=Cast<ACampusCameraPawn>(GetOwningPawn()); return A?FText::FromString(A->SaveStatus):FText::GetEmpty(); });
    auto Shell=SNew(SOverlay);
    auto Overlay=Shell;
    Shell->AddSlot()[BuildHeritageHUD()];
    auto Menu=SNew(SVerticalBox);
    Menu->AddSlot().AutoHeight().Padding(0,0,0,16)[Label(Text(TEXT("CAMPUS EN PAUSE")),22)];
    Menu->AddSlot().AutoHeight().Padding(0,4)[Button(TEXT("Reprendre la partie"),[this]() { ToggleMenu(); if(auto* C=Actor<ACampusClock>(GetWorld()); C && C->IsSimulationPaused()) { C->TogglePause(); } })];
    Menu->AddSlot().AutoHeight().Padding(0,4)[Button(TEXT("Sauvegarder"),[this]() { if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->SaveCampus(); } })];
    Menu->AddSlot().AutoHeight().Padding(0,4)[Button(TEXT("Charger - remplace la partie actuelle"),[this]() { if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->LoadCampus(); } })];
    Menu->AddSlot().AutoHeight().Padding(0,4)[Button(TEXT("Charger la copie precedente"),[this]() { if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->LoadBackup(); } })];
    Menu->AddSlot().AutoHeight().Padding(0,12)[Label(Text(TEXT("ZQSD : deplacer / clic droit : tourner / molette : zoom\nB : gymnase / C : chemins / R : tourner le placement\nP : planning / Espace : pause / 1, 2, 3 : vitesses\nF5 : sauvegarder / F9 : charger / F10 : copie precedente\nEchap : fermer ce menu en restant en pause")))];
    Menu->AddSlot().AutoHeight().Padding(0,8)[Label(Status)];
    Menu->AddSlot().AutoHeight().Padding(0,4)[Button(TEXT("Quitter la session"),[this]() { bConfirmQuit=true; })];
    Menu->AddSlot().AutoHeight().Padding(0,4)
    [SNew(SVerticalBox).Visibility_Lambda([this]() { return bConfirmQuit?EVisibility::Visible:EVisibility::Collapsed; })
        + SVerticalBox::Slot().AutoHeight()[Label(Text(TEXT("Les changements non sauvegardes seront perdus.")))]
        + SVerticalBox::Slot().AutoHeight().Padding(0,4)[Button(TEXT("Confirmer : quitter sans sauvegarder"),[this]() { UKismetSystemLibrary::QuitGame(this,GetOwningPlayerController(),EQuitPreference::Quit,false); })]
        + SVerticalBox::Slot().AutoHeight()[Button(TEXT("Annuler"),[this]() { bConfirmQuit=false; })]
    ];
    Overlay->AddSlot()
    [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.7f))
        .Visibility_Lambda([this]() { return bMenuOpen?EVisibility::Visible:EVisibility::Collapsed; })
        .OnMouseButtonDown_Lambda([](const FGeometry&,const FPointerEvent&) { return FReply::Handled(); })
        .HAlign(HAlign_Center).VAlign(VAlign_Center)
        [SNew(SBox).WidthOverride(560).MaxDesiredHeight_Lambda([this]() { FVector2D V; GetWorld()->GetGameViewport()->GetViewportSize(V); return FMath::Max(240.f,static_cast<float>(V.Y)-50); })
            [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Panel).Padding(24)
                [SNew(SScrollBox)+SScrollBox::Slot()[Menu]]]]
    ];
    auto Root=SNew(SCampusInterface).HUD(this); Root->SetContent(Shell);
    Shell->SetVisibility(EVisibility::SelfHitTestInvisible);
    Root->SetVisibility(EVisibility::SelfHitTestInvisible); Overlay->SetVisibility(EVisibility::SelfHitTestInvisible);
    Interface=Root; Viewport->AddViewportWidgetContent(Root,5);
    auto Keys=MakeShared<FCampusMenuInput>(this);
    Keys->HasInterfaceFocus=[this]()
    { return (Interface.IsValid() && (Interface->HasKeyboardFocus() || Interface->HasFocusedDescendants()))
        || (PlanningPanel.IsValid() && (PlanningPanel->HasKeyboardFocus() || PlanningPanel->HasFocusedDescendants())); };
    MenuInput=Keys;
    FSlateApplication::Get().RegisterInputPreProcessor(MenuInput,0);
}

void ACampusHUD::ToggleMenu()
{
    ClosePlanning(); bMenuOpen=!bMenuOpen; bConfirmQuit=false;
    if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->CancelTools(); }
    if(bMenuOpen)
    {
        if(auto* C=Actor<ACampusClock>(GetWorld()); C && !C->IsSimulationPaused()) { C->TogglePause(); }
        FSlateApplication::Get().SetKeyboardFocus(Interface,EFocusCause::SetDirectly);
    }
    else { FSlateApplication::Get().SetAllUserFocusToGameViewport(); }
}
void ACampusHUD::OpenPage(int32 Page)
{
    if(bMenuOpen) { return; }
    if(!PlanningPanel.IsValid()) { TogglePlanning(); }
    if(PlanningPanel.IsValid()) { PlanningPanel->SetPage(Page); }
}
void ACampusHUD::OpenAlert()
{
    auto* B=Actor<ACampusBuilding>(GetWorld());
    if(B && !B->IsConnected()) { if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->InterfaceAction(1); } return; }
    OpenPage(0); if(PlanningPanel.IsValid()) { PlanningPanel->SelectFirstAlert(); }
}
void ACampusHUD::TogglePlanning()
{
    if(bMenuOpen) { return; }
    if (PlanningPanel.IsValid()) { ClosePlanning(); return; }
    if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->CancelTools(); }
    UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
    if (!Viewport) { return; }
    ACampusBuilding* Gym = nullptr;
    for (TActorIterator<ACampusBuilding> It(GetWorld()); It; ++It) { Gym = *It; break; }
    if (!Gym || !Gym->IsBuilt()) { if(auto* A=Cast<ACampusCameraPawn>(GetOwningPawn())) { A->SaveStatus=TEXT("Construisez le gymnase pour ouvrir sa gestion. Bouton Gymnase ou B."); } return; }
    SAssignNew(PlanningPanel, SCampusPlanningPanel)
        .Building(Gym)
        .OnClose(FSimpleDelegate::CreateUObject(this, &ACampusHUD::ClosePlanning));
    Viewport->AddViewportWidgetContent(PlanningPanel.ToSharedRef(), 20);
    FSlateApplication::Get().SetKeyboardFocus(PlanningPanel, EFocusCause::SetDirectly);
}

void ACampusHUD::ClosePlanning()
{
    if (!PlanningPanel.IsValid()) { return; }
    if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
    {
        Viewport->RemoveViewportWidgetContent(PlanningPanel.ToSharedRef());
    }
    PlanningPanel.Reset();
    if (FSlateApplication::IsInitialized()) { FSlateApplication::Get().SetAllUserFocusToGameViewport(); }
}

void ACampusHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClosePlanning();
    if(Interface.IsValid() && GetWorld()->GetGameViewport()) { GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Interface.ToSharedRef()); }
    Interface.Reset();
    if(MenuInput.IsValid() && FSlateApplication::IsInitialized()) { FSlateApplication::Get().UnregisterInputPreProcessor(MenuInput); }
    MenuInput.Reset();
    Super::EndPlay(EndPlayReason);
}
