#include "SCampusPlanningPanel.h"
#include "CampusCameraPawn.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "CampusClock.h"
#include "CampusHUD.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "CampusBuilding.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SWrapBox.h"

namespace
{
    const TCHAR* PlanningDays[] = {TEXT("Lundi"), TEXT("Mardi"), TEXT("Mercredi"), TEXT("Jeudi"), TEXT("Vendredi"), TEXT("Samedi"), TEXT("Dimanche")};
    const FLinearColor Ink(.88f, .94f, .91f);
    const FLinearColor Muted(.57f, .68f, .63f);
    const TCHAR* ActivityNames[] = {TEXT("Cours universitaires"), TEXT("Entrainement"), TEXT("Acces libre"), TEXT("Cours public"), TEXT("Location club")};
    const TCHAR* ActivityShortNames[] = {TEXT("Universite"), TEXT("Entrainement"), TEXT("Acces libre"), TEXT("Cours public"), TEXT("Location")};
    const TCHAR* GroupNames[] = {TEXT("Sans groupe etudiant"), TEXT("Etudiants A (24)"), TEXT("Equipe basket (16)")};
    FVector2D ViewportSize()
    {
        FVector2D Size(1280, 720);
        if (GEngine && GEngine->GameViewport) { GEngine->GameViewport->GetViewportSize(Size); }
        return Size;
    }
}

void SCampusPlanningPanel::Construct(const FArguments& Args)
{
    Close = Args._OnClose;
    Building = Args._Building;
    for (const TCHAR* Name : ActivityNames) { ActivityOptions.Add(MakeShared<FString>(Name)); }
    for (const TCHAR* Name : GroupNames) { GroupOptions.Add(MakeShared<FString>(Name)); }
    StaffOptions.Add(MakeShared<FString>(TEXT("Sans encadrant")));
    StaffOptions.Add(MakeShared<FString>(TEXT("Camille Martin")));
    ContractOptions.Add(MakeShared<FString>(TEXT("Sans contrat")));
    ContractOptions.Add(MakeShared<FString>(TEXT("Club vendredi 20-22")));
    for (int32 Price = 0; Price <= 30; ++Price) { PriceOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%d EUR / personne"), Price))); }
    for (const TCHAR* Day : PlanningDays) { DayOptions.Add(MakeShared<FString>(Day)); }
    for (int32 Hour = 8; Hour < 22; ++Hour) { HourOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%02d:00"), Hour))); }
    for (int32 Duration = 1; Duration <= 14; ++Duration)
    {
        DurationOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%d h"), Duration)));
    }
    TSharedRef<SScrollBox> Vertical = SNew(SScrollBox)
        + SScrollBox::Slot()[SAssignNew(TableHost,SBox)[BuildTimetable()]];

    ChildSlot.HAlign(HAlign_Center).VAlign(VAlign_Center)
    [SNew(SBox)
        .WidthOverride_Lambda([]() { return FMath::Clamp(ViewportSize().X - 64.0f, 320.0f, 1180.0f); })
        .HeightOverride_Lambda([]() { return FMath::Clamp(ViewportSize().Y - 64.0f, 260.0f, 740.0f); })
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(.025f, .045f, .037f)).Padding(20)
            [SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
                [SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1)
                    [SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(bManagementOpen ? (ManagementPage==2 ? TEXT("Finances du campus") : TEXT("Personnel et contrats")) : TEXT("Planning du gymnase")); })
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 20)).ColorAndOpacity(Ink)]
                    + SHorizontalBox::Slot().AutoWidth()
                    [SNew(SButton).Text_Lambda([this]() { return FText::FromString(bManagementOpen ? TEXT("Retour au planning") : TEXT("Personnel et contrats")); })
                        .OnClicked_Lambda([this]() { bManagementOpen = !bManagementOpen; ManagementPage=1; return FReply::Handled(); })]
                    + SHorizontalBox::Slot().AutoWidth().Padding(6,0)
                    [SNew(SButton).Text(FText::FromString(TEXT("Finances"))).OnClicked_Lambda([this]() { SetPage(2); return FReply::Handled(); })]
                    + SHorizontalBox::Slot().AutoWidth().Padding(6,0)
                    [SNew(SButton).Text(FText::FromString(TEXT("Personnel"))).OnClicked_Lambda([this]() { SetPage(1); return FReply::Handled(); })]
                    + SHorizontalBox::Slot().AutoWidth()
                    [SNew(SButton).Text(FText::FromString(TEXT("Fermer [P]")))
                        .OnClicked_Lambda([this]() { Close.ExecuteIfBound(); return FReply::Handled(); })]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 16)
                [SNew(STextBlock).Text_Lambda([this]()
                    {
                        const int32 Count = Building.IsValid() ? Building->GetSchedule().GetReservationCount() : 0;
                        const int32 Hours = Building.IsValid() ? Building->GetSchedule().GetBookedHours() : 0;
                        const int32 Prep = Building.IsValid() ? Building->GetSchedule().GetPreparationHours() : 0;
                        return FText::FromString(FString::Printf(TEXT("Semaine type / %d reservation(s) / Activites : %d h (%.1f %%) / Preparation : %d h / Total : %d/98 h (%.1f %%)"),
                            Count, Hours, Hours * 100.0 / 98.0, Prep, Hours + Prep, (Hours + Prep) * 100.0 / 98.0));
                    })
                    .AutoWrapText(true).ColorAndOpacity(Muted)]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [SNew(STextBlock).Text(this, &SCampusPlanningPanel::GetNeedsText).AutoWrapText(true).ColorAndOpacity(Ink)
                    .Visibility_Lambda([this]() { return bManagementOpen ? EVisibility::Collapsed : EVisibility::Visible; })]
                + SVerticalBox::Slot().FillHeight(1)
                [SNew(SHorizontalBox).Visibility_Lambda([this]() { return bManagementOpen?EVisibility::Collapsed:EVisibility::Visible; })
                    + SHorizontalBox::Slot().FillWidth(1)
                    [SNew(SScrollBox).Orientation(Orient_Horizontal)
                    .Visibility_Lambda([this]() { return bManagementOpen ? EVisibility::Collapsed : EVisibility::Visible; })
                    + SScrollBox::Slot()
                    [SNew(SBox).MinDesiredWidth(620)
                        .WidthOverride_Lambda([]() { return FMath::Max(620.0f, FMath::Min(ViewportSize().X - 480.0f, 780.0f)); })
                        [Vertical]
                    ]
                ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(14,0,0,0)
                    [SNew(SBox).WidthOverride(320)
                        [SNew(SScrollBox)+SScrollBox::Slot()[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
                [SNew(STextBlock).Text(this, &SCampusPlanningPanel::GetSelectionText).AutoWrapText(true).ColorAndOpacity(Ink)
                    .Visibility_Lambda([this]() { return bManagementOpen ? EVisibility::Collapsed : EVisibility::Visible; })]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
                [SNew(SWrapBox).UseAllottedSize(true)
                    .Visibility_Lambda([this]()
                    {
                        return !bManagementOpen && Building.IsValid() && SelectedDay != INDEX_NONE
                            ? EVisibility::Visible : EVisibility::Collapsed;
                    })
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [MakeChoice(TEXT("Activite"), ActivityOptions, DraftActivity, 0, 180, ActivityChoice)]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [MakeChoice(TEXT("Groupe"), GroupOptions, DraftGroup, 0, 175, GroupChoice)]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [MakeChoice(TEXT("Jour"), DayOptions, DraftDay, 0, 105, DayChoice)]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [MakeChoice(TEXT("Debut"), HourOptions, DraftHour, 8, 80, HourChoice)]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [MakeChoice(TEXT("Duree"), DurationOptions, DraftDuration, 1, 70, DurationChoice)]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [MakeChoice(TEXT("Personnel"), StaffOptions, DraftStaff, 0, 145, StaffChoice)]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [SNew(SBox).Visibility_Lambda([this]() { return DraftActivity == 3 ? EVisibility::Visible : EVisibility::Collapsed; })
                        [MakeChoice(TEXT("Prix par seance"), PriceOptions, DraftPrice, 0, 150, PriceChoice)]]
                    + SWrapBox::Slot().Padding(0, 0, 12, 4)
                    [SNew(SBox).Visibility_Lambda([this]() { return DraftActivity == 4 ? EVisibility::Visible : EVisibility::Collapsed; })
                        [MakeChoice(TEXT("Location"), ContractOptions, DraftContract, 0, 175, ContractChoice)]]
                    + SWrapBox::Slot().VAlign(VAlign_Bottom).Padding(0, 0, 0, 4)
                    [SNew(SButton).Text_Lambda([this]() { return FText::FromString(EditingId ? TEXT("Enregistrer") : TEXT("Creer la reservation")); })
                        .OnClicked(this, &SCampusPlanningPanel::CreateReservation)]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                [SNew(SWrapBox).UseAllottedSize(true)
                    .Visibility_Lambda([this]() { return !bManagementOpen && EditingId ? EVisibility::Visible : EVisibility::Collapsed; })
                    + SWrapBox::Slot().Padding(0, 0, 12, 0)
                    [SNew(SButton).Text(FText::FromString(TEXT("Annuler les changements")))
                        .OnClicked_Lambda([this]() { SelectCell(SelectedDay, SelectedHour); return FReply::Handled(); })]
                    + SWrapBox::Slot().Padding(0, 0, 12, 0)
                    [SNew(SButton).Text_Lambda([this]() { return FText::FromString(bConfirmDelete ? TEXT("Confirmer la suppression") : TEXT("Supprimer")); })
                        .OnClicked(this, &SCampusPlanningPanel::DeleteReservation)]
                    + SWrapBox::Slot()
                    [SNew(SButton).Text(FText::FromString(TEXT("Garder la reservation")))
                        .Visibility_Lambda([this]() { return bConfirmDelete ? EVisibility::Visible : EVisibility::Collapsed; })
                        .OnClicked_Lambda([this]() { bConfirmDelete = false; return FReply::Handled(); })]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
                [SNew(STextBlock).Text_Lambda([this]()
                    {
                        if (!BookingError.IsEmpty()) { return BookingError; }
                        if (bManagementOpen) { return FText::GetEmpty(); }
                        if(Building.IsValid() && !Building->IsConnected())
                        { return FText::FromString(TEXT("Alerte : gymnase non raccorde. Reliez l'entree du campus a la case cyan avec C. Les seances ne pourront pas etre assurees.")); }
                        if (SelectedDay != INDEX_NONE && CampusNeedsStaff(static_cast<ECampusActivity>(DraftActivity)) && DraftStaff == 0)
                        { return FText::FromString(TEXT("Alerte : sans encadrant, cette seance sera manquee et ne rapportera rien. Le planning reste enregistrable.")); }
                        if (SelectedDay != INDEX_NONE && DraftActivity == 4 && DraftContract == 0)
                        { return FText::FromString(TEXT("Alerte : sans contrat accepte et associe, la location sera manquee.")); }
                        if (SelectedDay != INDEX_NONE && (DraftActivity == 0 || DraftActivity == 1) && DraftDuration != 2)
                        {
                            return FText::FromString(TEXT("Alerte : seance autorisee, mais seuls les creneaux de 2 h couvrent l'objectif du groupe."));
                        }
                        if (SelectedDay == INDEX_NONE) { return FText::FromString(TEXT("Cours public et location : 1 h de preparation avant la seance.")); }
                        const int32 Prep = CampusPreparationHours(static_cast<ECampusActivity>(DraftActivity));
                        return FText::FromString(Prep ? FString::Printf(TEXT("Brouillon : preparation %02d:00 - %02d:00 ; activite %02d:00 - %02d:00. Tout doit rester entre 08:00 et 22:00."),
                            DraftHour - Prep, DraftHour, DraftHour, DraftHour + DraftDuration)
                            : FString::Printf(TEXT("Brouillon : activite %02d:00 - %02d:00, sans preparation."), DraftHour, DraftHour + DraftDuration));
                    }).AutoWrapText(true)
                    .ColorAndOpacity(FLinearColor(1.0f, .55f, .4f))]
                        ]]
                    ]
                ]
                + SVerticalBox::Slot().FillHeight(1)
                [SNew(SBox).Visibility_Lambda([this]() { return bManagementOpen && ManagementPage==1 ? EVisibility::Visible : EVisibility::Collapsed; })[MakeManagement()]]
                + SVerticalBox::Slot().FillHeight(1)
                [SNew(SBox).Visibility_Lambda([this]() { return bManagementOpen && ManagementPage==2 ? EVisibility::Visible : EVisibility::Collapsed; })[MakeFinance()]]
            ]
        ]
    ];
}

TSharedRef<SWidget> SCampusPlanningPanel::MakeManagement()
{
    return SNew(SScrollBox)
        + SScrollBox::Slot()
        [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 8)
            [SNew(STextBlock).Text(FText::FromString(TEXT("Camille Martin / Enseignement, entrainement, cours public\nDisponibilite : lundi-vendredi 08-20 / Salaire : 600 EUR par semaine"))).AutoWrapText(true).ColorAndOpacity(Ink)]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
            [SNew(SButton).Text_Lambda([this]() { return FText::FromString(Building.IsValid() && Building->GetSchedule().IsStaffHired() ? TEXT("Camille recrutee") : TEXT("Recruter Camille - 600 EUR / semaine")); })
                .IsEnabled_Lambda([this]() { return Building.IsValid() && !Building->GetSchedule().IsStaffHired() && Building->GetOperations().Cash >= 0; })
                .OnClicked_Lambda([this]() { if (Building.IsValid() && Building->GetOperations().Cash >= 0) { Building->GetSchedule().HireStaff(); } return FReply::Handled(); })]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 8)
            [SNew(STextBlock).Text(FText::FromString(TEXT("Offre club : vendredi 20-22 / 20 participants / 300 EUR apres prestation\nAccepter, puis creer une location vendredi 20-22 et lui associer le contrat. Offre ponctuelle pour la semaine indiquee."))).AutoWrapText(true).ColorAndOpacity(Ink)]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
            [SNew(SButton).Text_Lambda([this]() { return FText::FromString(FString::Printf(TEXT("Accepter l'offre - semaine %lld"), Building.IsValid() ? Building->GetOperations().OfferedWeek() + 1 : 1)); })
                .IsEnabled_Lambda([this]() { if (!Building.IsValid()) { return false; } const auto& O = Building->GetOperations(); return O.ContractState != 1 && O.ContractWeek != O.OfferedWeek(); })
                .OnClicked_Lambda([this]() { if (Building.IsValid()) { Building->GetOperations().AcceptContract(); } return FReply::Handled(); })]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 12)
            [SNew(STextBlock).Text_Lambda([this]() { if(!Building.IsValid()) { return FText::GetEmpty(); }
                const auto& S=Building->GetSchedule(); const auto& O=Building->GetOperations();
                const TCHAR* State=O.ContractState==1?(S.HasContractBooking()?TEXT("Accepte et programme"):TEXT("Accepte : reservation manquante")):O.ContractState==2?TEXT("Honore"):O.ContractState==3?TEXT("Manque"):TEXT("Disponible");
                return FText::FromString(FString::Printf(TEXT("Charge de Camille : %d / 60 h\nSeances sans encadrant : %d\nContrat : %s"),S.GetStaffHours(),S.GetUnstaffedCount(),State));
            }).AutoWrapText(true).ColorAndOpacity(Ink)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,8)[SNew(SButton).Text(FText::FromString(TEXT("Ouvrir une seance a corriger"))).OnClicked_Lambda([this]() { SelectFirstAlert(); return FReply::Handled(); })]
        ];
}

TSharedRef<SWidget> SCampusPlanningPanel::MakeChoice(const TCHAR* Label, TArray<TSharedPtr<FString>>& Options,
    int32& Value, int32 Offset, float Width, TSharedPtr<SComboBox<TSharedPtr<FString>>>& Choice)
{
    // Options and Value refer to members of this panel, never temporary form data.
    auto* Values = &Options;
    auto* Draft = &Value;
    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(Label)).ColorAndOpacity(Muted)]
        + SVerticalBox::Slot().AutoHeight()
        [SNew(SBox).WidthOverride(Width)
            [SAssignNew(Choice, SComboBox<TSharedPtr<FString>>).OptionsSource(Values).InitiallySelectedItem(Options[Value - Offset])
                .OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) { return SNew(STextBlock).Text(FText::FromString(*Item)); })
                .OnSelectionChanged_Lambda([this, Values, Draft, Offset](TSharedPtr<FString> Item, ESelectInfo::Type)
                {
                    const int32 Index = Values->IndexOfByKey(Item);
                    if (Index != INDEX_NONE)
                    {
                        *Draft = Index + Offset; BookingError = FText::GetEmpty(); bConfirmDelete = false;
                        if (Draft == &DraftActivity && GroupChoice.IsValid())
                        {
                            DraftGroup = static_cast<int32>(CampusDefaultGroup(static_cast<ECampusActivity>(DraftActivity)));
                            GroupChoice->SetSelectedItem(GroupOptions[DraftGroup]);
                            if (StaffChoice.IsValid() && !CampusNeedsStaff(static_cast<ECampusActivity>(DraftActivity)))
                            { DraftStaff = 0; StaffChoice->SetSelectedItem(StaffOptions[0]); }
                            if (ContractChoice.IsValid() && DraftActivity != 4)
                            { DraftContract = 0; ContractChoice->SetSelectedItem(ContractOptions[0]); }
                        }
                    }
                })
                [SNew(STextBlock).Text_Lambda([Values, Draft, Offset]() { return FText::FromString(*(*Values)[*Draft - Offset]); })]
            ]
        ];
}

void SCampusPlanningPanel::SelectCell(int32 Day, int32 Hour)
{
    SelectedDay = Day;
    SelectedHour = Hour;
    BookingError = FText::GetEmpty();
    bConfirmDelete = false;
    const auto* R = GetReservationAt(Day, Hour);
    EditingId = R ? R->Id : 0;
    DraftDay = Day;
    DraftHour = R ? R->StartHour : Hour;
    DraftActivity = R ? static_cast<int32>(R->Activity) : 0;
    DraftDuration = R ? R->Duration : 1;
    DayChoice->SetSelectedItem(DayOptions[DraftDay]);
    HourChoice->SetSelectedItem(HourOptions[DraftHour - 8]);
    ActivityChoice->SetSelectedItem(ActivityOptions[DraftActivity]);
    DraftGroup = R ? static_cast<int32>(R->Group) : static_cast<int32>(CampusDefaultGroup(static_cast<ECampusActivity>(DraftActivity)));
    GroupChoice->SetSelectedItem(GroupOptions[DraftGroup]);
    DurationChoice->SetSelectedItem(DurationOptions[DraftDuration - 1]);
    DraftStaff = R ? R->StaffId : 0;
    DraftPrice = R ? R->Price : 12;
    DraftContract = R && R->Contract ? 1 : 0;
    StaffChoice->SetSelectedItem(StaffOptions[DraftStaff]);
    PriceChoice->SetSelectedItem(PriceOptions[DraftPrice]);
    ContractChoice->SetSelectedItem(ContractOptions[DraftContract]);
}

FText SCampusPlanningPanel::GetNeedsText() const
{
    if (!Building.IsValid()) { return FText::GetEmpty(); }
    const auto Students = Building->GetSchedule().GetNeeds(ECampusGroup::Students);
    const auto Team = Building->GetSchedule().GetNeeds(ECampusGroup::Team);
    const auto Line = [](const TCHAR* Name, const FCampusGroupNeeds& Needs)
    {
        const FString Status = Needs.MissingSessions() ? FString::Printf(TEXT("Manque %d seance(s)"), Needs.MissingSessions())
            : TEXT("Objectif couvert");
        return FString::Printf(TEXT("%s : %d h prevues / %d h visees ; %d/%d seances de 2 h - %s"),
            Name, Needs.PlannedHours, Needs.RequiredSessions * 2, Needs.PlannedSessions, Needs.RequiredSessions, *Status);
    };
    return FText::FromString(Line(GroupNames[1], Students) + TEXT("\n") + Line(GroupNames[2], Team));
}

FText SCampusPlanningPanel::GetSelectionText() const
{
    if (SelectedDay == INDEX_NONE)
    {
        return FText::FromString(TEXT("08:00 - 22:00  /  Cliquez sur une case pour consulter son horaire."));
    }
    const auto* R = GetReservationAt(SelectedDay, SelectedHour);
    const FString Group = R ? FString::Printf(TEXT(" / %s"), GroupNames[static_cast<int32>(R->Group)]) : FString();
    const FString Staff = R && CampusNeedsStaff(R->Activity) ? (R->StaffId ? TEXT(" / Camille Martin") : TEXT(" / SANS ENCADRANT")) : TEXT("");
    return FText::FromString(GetCellTooltip(SelectedDay, SelectedHour).ToString() + Group + Staff
        + TEXT(" / Les changements concernent les prochaines occurrences ; une seance commencee conserve ses conditions."));
}

const FCampusReservation* SCampusPlanningPanel::GetReservationAt(int32 Day, int32 Hour) const
{
    return Building.IsValid() ? Building->GetSchedule().FindAt(Day, Hour) : nullptr;
}

FText SCampusPlanningPanel::GetCellText(int32 Day, int32 Hour) const
{
    const auto* Reservation = GetReservationAt(Day, Hour);
    if (Reservation && Hour < Reservation->StartHour) { return FText::FromString(TEXT("Preparation")); }
    return Reservation ? FText::FromString(ActivityShortNames[static_cast<int32>(Reservation->Activity)]) : FText::GetEmpty();
}

FText SCampusPlanningPanel::GetCellTooltip(int32 Day, int32 Hour) const
{
    if (const auto* R = GetReservationAt(Day, Hour))
    {
        if (R->PreparationHours())
        {
            return FText::FromString(FString::Printf(TEXT("%s / %s / Preparation %02d:00 - %02d:00 / Activite %02d:00 - %02d:00 / Chaque semaine"),
                PlanningDays[Day], ActivityNames[static_cast<int32>(R->Activity)], R->OccupiedStartHour(), R->StartHour, R->StartHour, R->EndHour()));
        }
        return FText::FromString(FString::Printf(TEXT("%s  %02d:00 - %02d:00  /  %s  /  Chaque semaine"),
            PlanningDays[Day], R->StartHour, R->EndHour(), ActivityNames[static_cast<int32>(R->Activity)]));
    }
    return FText::FromString(FString::Printf(TEXT("%s  %02d:00 - %02d:00  /  Disponible"), PlanningDays[Day], Hour, Hour + 1));
}

FLinearColor SCampusPlanningPanel::GetCellColor(int32 Day, int32 Hour) const
{
    const auto* SelectedReservation = GetReservationAt(Day, Hour);
    if (SelectedReservation && Hour < SelectedReservation->StartHour)
    {
        return SelectedReservation->Id == EditingId ? FLinearColor(.48f, .48f, .25f) : FLinearColor(.30f, .28f, .15f);
    }
    if ((EditingId && SelectedReservation && SelectedReservation->Id == EditingId)
        || (SelectedDay == Day && SelectedHour == Hour)) { return FLinearColor(.16f, .58f, .43f); }
    if (const auto* R = GetReservationAt(Day, Hour))
    {
        const FLinearColor Colors[] = {FLinearColor(.12f,.28f,.46f), FLinearColor(.30f,.19f,.43f),
            FLinearColor(.12f,.34f,.24f), FLinearColor(.46f,.29f,.10f), FLinearColor(.27f,.31f,.38f)};
        return Colors[static_cast<int32>(R->Activity)];
    }
    return FLinearColor(.09f, .14f, .12f);
}

FReply SCampusPlanningPanel::CreateReservation()
{
    if (!Building.IsValid() || SelectedDay == INDEX_NONE) { return FReply::Handled(); }
    auto& Schedule = Building->GetSchedule();
    const auto Activity = static_cast<ECampusActivity>(DraftActivity);
    int64 PlannedAt=0;
    for(TActorIterator<ACampusClock> It(Building->GetWorld());It;++It) { PlannedAt=It->GetTotalMinutes(); break; }
    if (DraftContract && Building->GetOperations().ContractState != 1)
    { BookingError = FText::FromString(TEXT("Acceptez d'abord l'offre dans Personnel et contrats.")); return FReply::Handled(); }
    const auto Result = EditingId ? Schedule.Update(EditingId, DraftDay, DraftHour, DraftDuration, Activity, DraftGroup, DraftStaff, DraftPrice, DraftContract != 0, PlannedAt)
        : Schedule.Add(DraftDay, DraftHour, DraftDuration, Activity, DraftGroup, DraftStaff, DraftPrice, DraftContract != 0, PlannedAt);
    switch (Result)
    {
    case ECampusBookingResult::Success:
        UE_LOG(LogTemp, Display, TEXT("CampusPlanning: reservation saved day %d at %d, duration %d"), DraftDay, DraftHour, DraftDuration);
        SelectCell(DraftDay, DraftHour);
        RefreshTimetable();
        break;
    case ECampusBookingResult::OutsideOpeningHours:
        BookingError = FText::FromString(TEXT("Activite et preparation doivent rester entre 08:00 et 22:00. Cours public/location : debut au plus tot a 09:00.")); break;
    case ECampusBookingResult::Occupied:
        if (const auto* Conflict = Schedule.FindConflict(DraftDay, DraftHour, DraftDuration, EditingId, Activity))
        {
            BookingError = FText::FromString(FString::Printf(TEXT("Conflit activite/preparation : %s, %s. Terrain occupe %02d:00 - %02d:00 (activite %02d:00 - %02d:00). Aucun changement enregistre."),
                ActivityNames[static_cast<int32>(Conflict->Activity)], PlanningDays[Conflict->Day], Conflict->OccupiedStartHour(), Conflict->EndHour(), Conflict->StartHour, Conflict->EndHour()));
        }
        else { BookingError = FText::FromString(TEXT("Le gymnase est deja reserve sur cet horaire.")); }
        break;
    case ECampusBookingResult::NotFound:
        BookingError = FText::FromString(TEXT("Cette reservation n'existe plus. Selectionnez une autre case.")); break;
    case ECampusBookingResult::InvalidGroup:
        BookingError = FText::FromString(TEXT("Groupe incompatible : Universite = Etudiants A ; Entrainement = Equipe basket ; Public/Location = Sans groupe etudiant. Acces libre : choix libre.")); break;
    case ECampusBookingResult::InvalidStaff:
        BookingError = FText::FromString(TEXT("Recrutez Camille dans Personnel et contrats. Affectation possible seulement aux cours universitaires, entrainements et cours publics.")); break;
    case ECampusBookingResult::StaffUnavailable:
        BookingError = FText::FromString(TEXT("Camille est disponible lundi-vendredi, 08:00-20:00. La seance doit tenir dans ces horaires.")); break;
    case ECampusBookingResult::InvalidPrice:
        BookingError = FText::FromString(TEXT("Le prix doit etre compris entre 0 et 30 EUR.")); break;
    case ECampusBookingResult::InvalidContract:
        BookingError = FText::FromString(TEXT("Le contrat exige une location vendredi 20:00-22:00, avec preparation 19:00-20:00.")); break;
    default:
        BookingError = FText::FromString(TEXT("Choisissez un jour, une activite et une duree valides.")); break;
    }
    return FReply::Handled();
}

FReply SCampusPlanningPanel::DeleteReservation()
{
    if (!Building.IsValid() || !EditingId) { return FReply::Handled(); }
    if (!bConfirmDelete) { bConfirmDelete = true; return FReply::Handled(); }
    const auto* R = Building->GetSchedule().FindById(EditingId);
    const int32 Day = R ? R->Day : SelectedDay;
    const int32 Hour = R ? R->StartHour : SelectedHour;
    Building->GetSchedule().Remove(EditingId);
    SelectCell(Day, Hour);
    RefreshTimetable();
    return FReply::Handled();
}

FReply SCampusPlanningPanel::OnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (Event.GetKey() == EKeys::P)
    {
        if (!Event.IsRepeat()) { Close.ExecuteIfBound(); }
        return FReply::Handled();
    }
    return SCompoundWidget::OnKeyDown(Geometry, Event);
}

FReply SCampusPlanningPanel::OnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    const FKey Key = Event.GetKey();
    if(Key==EKeys::Escape)
    {
        if(!Event.IsRepeat() && Building.IsValid()) { for(TActorIterator<ACampusHUD> It(Building->GetWorld());It;++It) { It->ToggleMenu(); break; } }
        return FReply::Handled();
    }
    if (Key == EKeys::F5 || Key == EKeys::F9 || Key == EKeys::F10)
    {
        if (!Event.IsRepeat() && Building.IsValid())
        {
            for (TActorIterator<ACampusCameraPawn> It(Building->GetWorld()); It; ++It)
            {
                if (Key == EKeys::F5) { It->SaveCampus(); }
                else if (Key == EKeys::F9) { It->LoadCampus(); }
                else { It->LoadBackup(); }
                break;
            }
        }
        return FReply::Handled();
    }
    // Intercept before a focused cell button treats Space as activation.
    if (Key == EKeys::P)
    {
        if (!Event.IsRepeat()) { Close.ExecuteIfBound(); }
        return FReply::Handled();
    }
    const int32 Speed = (Key == EKeys::One || Key == EKeys::NumPadOne || Key == EKeys::Ampersand) ? 1
        : (Key == EKeys::Two || Key == EKeys::NumPadTwo || Key == EKeys::E_AccentAigu) ? 2
        : (Key == EKeys::Three || Key == EKeys::NumPadThree || Key == EKeys::Quote) ? 4 : 0;
    if (Key == EKeys::SpaceBar || Speed != 0)
    {
        if (!Event.IsRepeat() && GEngine && GEngine->GameViewport)
        {
            for (TActorIterator<ACampusClock> It(GEngine->GameViewport->GetWorld()); It; ++It)
            {
                if (Speed) { It->SetSimulationSpeed(Speed); } else { It->TogglePause(); }
                break;
            }
        }
        return FReply::Handled();
    }
    return SCompoundWidget::OnPreviewKeyDown(Geometry, Event);
}

TSharedRef<SWidget> SCampusPlanningPanel::BuildTimetable()
{
    auto Grid=SNew(SGridPanel); Grid->SetColumnFill(0,0);
    for(int Day=0;Day<7;++Day)
    { Grid->SetColumnFill(Day+1,1); Grid->AddSlot(Day+1,0).Padding(4)[SNew(STextBlock).Text(FText::FromString(PlanningDays[Day])).ColorAndOpacity(Ink)]; }
    for(int H=8;H<22;++H)
    { Grid->AddSlot(0,H-7)[SNew(SBox).HeightOverride(32).WidthOverride(60)[SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%02d:00"),H))).ColorAndOpacity(Muted)]]; }
    for(int Day=0;Day<7;++Day) for(int H=8;H<22;)
    {
        const auto* R=GetReservationAt(Day,H);
        const int Span=R && H>=R->StartHour ? R->EndHour()-H : 1;
        Grid->AddSlot(Day+1,H-7).RowSpan(Span).Padding(1)
        [SNew(SBox).HeightOverride(32*Span-2)
            [SNew(SButton).ContentPadding(FMargin(6,2)).ToolTipText_Lambda([this,Day,H]() { return GetCellTooltip(Day,H); })
                .ButtonColorAndOpacity_Lambda([this,Day,H]() { return GetCellColor(Day,H); })
                .OnClicked_Lambda([this,Day,H]() { SelectCell(Day,H); return FReply::Handled(); })
                [SNew(STextBlock).Text_Lambda([this,Day,H]()
                {
                    const auto* Item=GetReservationAt(Day,H); if(!Item) { return FText::GetEmpty(); }
                    if(H<Item->StartHour) { return FText::FromString(TEXT("Preparation")); }
                    return FText::FromString(FString::Printf(TEXT("%s  %02d-%02d%s"),ActivityShortNames[static_cast<int32>(Item->Activity)],Item->StartHour,Item->EndHour(),
                        CampusNeedsStaff(Item->Activity) && !Item->StaffId ? TEXT(" / ! encadrant") : TEXT("")));
                }).AutoWrapText(true).Font(FCoreStyle::GetDefaultFontStyle("Regular",10)).ColorAndOpacity(Ink)]
            ]
        ]; H+=Span;
    }
    return Grid;
}
void SCampusPlanningPanel::RefreshTimetable() { if(TableHost.IsValid()) { TableHost->SetContent(BuildTimetable()); } }
void SCampusPlanningPanel::SelectFirstAlert()
{
    SetPage(0); if(!Building.IsValid()) { return; }
    for(int D=0;D<7;++D) for(int H=8;H<22;++H)
    {
        const auto* R=GetReservationAt(D,H); if(!R || R->StartHour!=H) { continue; }
        if((CampusNeedsStaff(R->Activity) && !R->StaffId) || (R->Activity==ECampusActivity::ClubRental && (!R->Contract || Building->GetOperations().ContractState!=1)))
        { SelectCell(D,H); return; }
    }
    if(Building->GetOperations().ContractState==1 && !Building->GetSchedule().HasContractBooking()) { SelectCell(4,20); return; }
    BookingError=FText::FromString(TEXT("Pas de seance sans encadrant ni de location a corriger. Consultez les besoins hebdomadaires ci-dessus."));
}
TSharedRef<SWidget> SCampusPlanningPanel::MakeFinance()
{
    auto Rows=SNew(SVerticalBox);
    auto Cell=[](TAttribute<FText> Text) { return SNew(STextBlock).Text(Text).ColorAndOpacity(Ink).AutoWrapText(true).Font(FCoreStyle::GetDefaultFontStyle("Regular",11)); };
    Rows->AddSlot().AutoHeight().Padding(0,8)[Cell(TAttribute<FText>::CreateLambda([this]()
    { if(!Building.IsValid()) { return FText::GetEmpty(); } const auto& O=Building->GetOperations();
        return FText::FromString(FString::Printf(TEXT("Tresorerie : %lld EUR     |     Investissements : %lld EUR\nLes investissements sont distincts du resultat d'exploitation."),O.Cash,O.Investment)); }))];
    auto Summary=SNew(SGridPanel);
    const TCHAR* Titles[]={TEXT("Exploitation"),TEXT("Cette semaine"),TEXT("Semaine precedente")};
    for(int C=0;C<3;++C) { Summary->SetColumnFill(C,1); Summary->AddSlot(C,0).Padding(8)[Cell(FText::FromString(Titles[C]))]; }
    const TCHAR* Labels[]={TEXT("Recettes"),TEXT("Depenses"),TEXT("Resultat"),TEXT("Seances assurees"),TEXT("Seances manquees")};
    for(int R=0;R<5;++R)
    {
        Summary->AddSlot(0,R+1).Padding(8)[Cell(FText::FromString(Labels[R]))];
        for(int C=1;C<3;++C) { Summary->AddSlot(C,R+1).Padding(8)[Cell(TAttribute<FText>::CreateLambda([this,R,C]()
        { if(!Building.IsValid()) { return FText::GetEmpty(); } const auto& O=Building->GetOperations();
            if(C==2 && !O.HasPreviousWeek) { return FText::FromString(TEXT("--")); }
            const auto& W=C==1?O.Week:O.PreviousWeek;
            const int64 V=R==0?W.Revenue:R==1?W.Expense:R==2?W.Profit():R==3?W.Completed:W.Missed;
            return FText::FromString(FString::Printf(TEXT("%lld%s"),V,R<3?TEXT(" EUR"):TEXT("")));
        }))]; }
    }
    Rows->AddSlot().AutoHeight()[Summary];
    Rows->AddSlot().AutoHeight().Padding(0,14)[Cell(FText::FromString(TEXT("JOURNAL / 128 dernieres ecritures / plus recentes en premier")))];
    const TCHAR* Head[]={TEXT("Date"),TEXT("Operation"),TEXT("Recette"),TEXT("Depense")};
    auto Header=SNew(SHorizontalBox);
    for(int C=0;C<4;++C) { Header->AddSlot().FillWidth(C==1?3:1).Padding(6)[Cell(FText::FromString(Head[C]))]; }
    Rows->AddSlot().AutoHeight()[Header];
    for(int I=0;I<128;++I)
    {
        auto Row=SNew(SHorizontalBox);
        for(int C=0;C<4;++C)
        {
            Row->AddSlot().FillWidth(C==1?3:1).Padding(6)
            [Cell(TAttribute<FText>::CreateLambda([this,I,C]()
            {
                if(!Building.IsValid() || I>=Building->GetOperations().EntryCount()) { return FText::GetEmpty(); }
                const auto& E=Building->GetOperations().RecentEntry(I);
                if(C==0) { return FText::FromString(FString::Printf(TEXT("S%lld %s %02lld h"),E.Hour/168+1,PlanningDays[E.Hour/24%7],E.Hour%24)); }
                if(C==2 || C==3) { return FText::FromString(FString::Printf(TEXT("%d EUR"),C==2?E.Revenue:E.Expense)); }
                const TCHAR* Names[]={TEXT("Seance assuree"),TEXT("Manquee : encadrant absent"),TEXT("Manquee : contrat absent"),TEXT("Manquee : demande nulle"),TEXT("Charges gymnase"),TEXT("Salaire Camille"),TEXT("Forfait universitaire"),TEXT("Contrat manque"),TEXT("Manquee : salle occupee"),TEXT("Investissement : gymnase"),TEXT("Investissement : chemins"),TEXT("Manquee : inaccessible"),TEXT("Manquee : arrivee tardive"),TEXT("Location partielle")};
                FString Detail=Names[static_cast<int>(E.Kind)];
                if(E.MinutesAssured || E.DelayMinutes) { Detail+=FString::Printf(TEXT(" / %d min assurees / retard %d min"),E.MinutesAssured,E.DelayMinutes); }
                return FText::FromString(Detail);
            }))];
        }
        Rows->AddSlot().AutoHeight()
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(I%2?FLinearColor(.035f,.065f,.07f):FLinearColor(.055f,.085f,.09f)).Padding(0)
            .Visibility_Lambda([this,I]() { return Building.IsValid() && I<Building->GetOperations().EntryCount()?EVisibility::Visible:EVisibility::Collapsed; })[Row]];
    }
    return SNew(SScrollBox)+SScrollBox::Slot()[Rows];
}
