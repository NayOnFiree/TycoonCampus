#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CampusHUD.generated.h"

class ACampusClock;
class SCampusPlanningPanel;
class SWidget;
class IInputProcessor;

UCLASS()
class TYCOONCAMPUS_API ACampusHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;
    void OpenPage(int32 Page);
    void ToggleMenu();
    bool IsMenuOpen() const { return bMenuOpen; }
    bool IsModalOpen() const { return bMenuOpen || IsPlanningOpen(); }
    void OpenAlert();
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void TogglePlanning();
    bool IsPlanningOpen() const { return PlanningPanel.IsValid(); }
private:
    void ClosePlanning();
    void BuildInterface();
    TSharedRef<SWidget> BuildHeritageHUD();
    TSharedPtr<SWidget> Interface;
    TSharedPtr<IInputProcessor> MenuInput;
    bool bMenuOpen=false;
    bool bConfirmQuit=false;
    TSharedPtr<SCampusPlanningPanel> PlanningPanel;
    TWeakObjectPtr<ACampusClock> Clock;
};
