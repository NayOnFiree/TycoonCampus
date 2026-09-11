#pragma once
#include "Widgets/SCompoundWidget.h"
#include "CampusWeeklySchedule.h"
#include "Widgets/Input/SComboBox.h"

class ACampusBuilding;
class SBox;

/** Recurring timetable with transactional reservation editing. */
class SCampusPlanningPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCampusPlanningPanel) {}
        SLATE_EVENT(FSimpleDelegate, OnClose)
        SLATE_ARGUMENT(TWeakObjectPtr<ACampusBuilding>, Building)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    void SetPage(int32 Page) { bManagementOpen=Page!=0; ManagementPage=Page; }
    void SelectFirstAlert();
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    virtual FReply OnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    virtual FReply OnMouseButtonDown(const FGeometry&, const FPointerEvent&) override { return FReply::Handled(); }
    virtual FReply OnMouseWheel(const FGeometry&, const FPointerEvent&) override { return FReply::Handled(); }
private:
    TSharedPtr<SBox> TableHost;
    TSharedRef<SWidget> BuildTimetable();
    void RefreshTimetable();
    FSimpleDelegate Close;
    TWeakObjectPtr<ACampusBuilding> Building;
    TArray<TSharedPtr<FString>> ActivityOptions;
    TArray<TSharedPtr<FString>> DurationOptions;
    TArray<TSharedPtr<FString>> DayOptions;
    TArray<TSharedPtr<FString>> HourOptions;
    TArray<TSharedPtr<FString>> GroupOptions;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> GroupChoice;
    int32 DraftGroup = 1;
    TArray<TSharedPtr<FString>> StaffOptions, PriceOptions, ContractOptions;
    TSharedPtr<SComboBox<TSharedPtr<FString>>> StaffChoice, PriceChoice, ContractChoice;
    int32 DraftStaff = 0, DraftPrice = 12, DraftContract = 0;
    bool bManagementOpen = false;
    int32 ManagementPage=1;
    TSharedRef<SWidget> MakeFinance();
    TSharedRef<SWidget> MakeManagement();
    TSharedPtr<SComboBox<TSharedPtr<FString>>> ActivityChoice, DurationChoice, DayChoice, HourChoice;
    int32 DraftActivity = 0;
    int32 DraftDuration = 1;
    int32 DraftDay = 0;
    int32 DraftHour = 8;
    std::int64_t EditingId = 0;
    bool bConfirmDelete = false;
    FText BookingError;
    int32 SelectedDay = INDEX_NONE;
    int32 SelectedHour = INDEX_NONE;
    FText GetSelectionText() const;
    FText GetNeedsText() const;
    const FCampusReservation* GetReservationAt(int32 Day, int32 Hour) const;
    FText GetCellText(int32 Day, int32 Hour) const;
    FText GetCellTooltip(int32 Day, int32 Hour) const;
    FLinearColor GetCellColor(int32 Day, int32 Hour) const;
    FReply CreateReservation();
    FReply DeleteReservation();
    void SelectCell(int32 Day, int32 Hour);
    TSharedRef<SWidget> MakeChoice(const TCHAR* Label, TArray<TSharedPtr<FString>>& Options,
        int32& Value, int32 Offset, float Width, TSharedPtr<SComboBox<TSharedPtr<FString>>>& Choice);
};
