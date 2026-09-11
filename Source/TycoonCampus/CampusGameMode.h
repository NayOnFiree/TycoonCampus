#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CampusGameMode.generated.h"

UCLASS()
class TYCOONCAMPUS_API ACampusGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ACampusGameMode();
protected:
    virtual void BeginPlay() override;
};
