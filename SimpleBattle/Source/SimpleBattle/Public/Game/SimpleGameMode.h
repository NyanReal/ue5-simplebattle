// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SimpleGameMode.generated.h"

/**
 * Default game mode for SimpleBattle.
 * Sets the default pawn and player controller classes.
 */
UCLASS()
class SIMPLEBATTLE_API ASimpleGameMode : public AGameModeBase {
  GENERATED_BODY()

public:
  ASimpleGameMode();

protected:
  virtual void BeginPlay() override;
};
