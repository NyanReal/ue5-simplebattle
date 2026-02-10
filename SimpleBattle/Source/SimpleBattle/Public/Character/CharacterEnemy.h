// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterEnemy.generated.h"


class UStaticMeshAppearanceComponent;
class UAttackWarningComponent;

/**
 * Enemy character that displays a rectangular decal warning before attacking.
 * Controlled by EnemyAIController via code.
 */
UCLASS()
class SIMPLEBATTLE_API ACharacterEnemy : public ACharacter {
  GENERATED_BODY()

public:
  ACharacterEnemy();

  /** Show the forward rectangular attack warning decal. */
  void ShowAttackWarning();

  /** Hide the attack warning decal. */
  void HideAttackWarning();

protected:
  virtual void BeginPlay() override;

private:
  // --- Components ---
  UPROPERTY(VisibleAnywhere, Category = "Appearance")
  TObjectPtr<UStaticMeshAppearanceComponent> AppearanceComp;

  UPROPERTY(VisibleAnywhere, Category = "AttackWarning")
  TObjectPtr<UAttackWarningComponent> AttackWarningComp;
};
