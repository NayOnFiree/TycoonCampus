#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CampusToolMode.h"
#include "CampusHUD.generated.h"

class ACampusClock;
class SCampusPlanningPanel;
class SWidget;
class IInputProcessor;

UCLASS()
class TYCOONCAMPUS_API ACampusHUD : public AHUD
{
    GENERATED_BODY()
    friend class ACampusCameraPawn;
public:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;
    void OpenPage(int32 Page);
    void ToggleMenu();
    void SetConstructionType(int32 TypeIndex);
    int32 GetConstructionType() const { return ConstructionType; }
    bool IsConstructionTypeAvailable(int32 TypeIndex) const;
    FText GetConstructionTypeName() const;
    void HandleEscape();
    bool IsMenuOpen() const { return bMenuOpen; }
    bool IsModalOpen() const { return bMenuOpen || IsPlanningOpen(); }
    void OpenAlert();
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void TogglePlanning();
    bool IsPlanningOpen() const { return PlanningPanel.IsValid(); }
private:
    bool ApplyToolMode(ECampusToolMode Mode);
    void RequestClosePlanning();
    void ClosePlanning();
    void BuildInterface();
    TSharedRef<SWidget> BuildHeritageHUD();
    TSharedPtr<SWidget> Interface;
    TSharedPtr<IInputProcessor> MenuInput;
    bool bMenuOpen=false;
    bool bConfirmQuit=false;
    TSharedPtr<SCampusPlanningPanel> PlanningPanel;
    TWeakObjectPtr<ACampusClock> Clock;
    int32 ConstructionType = 0;
};
