// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimplePlayerController.generated.h"

/**
 * Player controller that enables mouse cursor display for top-down gameplay.
 */
UCLASS()
class SIMPLEBATTLE_API ASimplePlayerController : public APlayerController {
  GENERATED_BODY()

protected:
  virtual void BeginPlay() override;
};
