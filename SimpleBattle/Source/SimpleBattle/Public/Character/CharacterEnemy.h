// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterEnemy.generated.h"

class UStaticMeshAppearanceComponent;
class UDecalComponent;
class UMaterialInstanceDynamic;

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
  virtual void Tick(float DeltaTime) override;

private:
  // --- Components ---
  UPROPERTY(VisibleAnywhere, Category = "Appearance")
  TObjectPtr<UStaticMeshAppearanceComponent> AppearanceComp;

  UPROPERTY(VisibleAnywhere, Category = "AttackWarning")
  TObjectPtr<UDecalComponent> WarningDecalComp;

  // --- Decal Settings ---

  /** Length of the warning rectangle (forward direction). */
  UPROPERTY(EditAnywhere, Category = "AttackWarning")
  float WarningLength = 500.f;

  /** Width of the warning rectangle. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning")
  float WarningWidth = 150.f;

  /** Forward offset from character center. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning")
  float WarningForwardOffset = 250.f;

  /** Duration of the wipe reveal animation in seconds. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning")
  float WipeAnimDuration = 0.4f;

  UPROPERTY()
  TObjectPtr<UMaterialInstanceDynamic> DecalDynamicMaterial;

  // --- Wipe animation state ---
  bool bIsWipeAnimating = false;
  float WipeProgress = 0.f;

  /** Create the decal material programmatically. */
  void CreateDecalMaterial();
};
