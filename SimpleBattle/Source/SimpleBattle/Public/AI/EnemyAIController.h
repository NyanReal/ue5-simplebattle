// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "EnemyAIController.generated.h"


class ACharacterEnemy;

/** Enemy AI states. */
UENUM()
enum class EEnemyState : uint8 { Idle, Moving, AttackWarning };

/**
 * Simple AI controller that cycles: Idle -> Move toward player -> Attack
 * Warning -> Idle.
 */
UCLASS()
class SIMPLEBATTLE_API AEnemyAIController : public AAIController {
  GENERATED_BODY()

public:
  AEnemyAIController();

protected:
  virtual void OnPossess(APawn *InPawn) override;
  virtual void Tick(float DeltaTime) override;

private:
  // --- State ---
  EEnemyState CurrentState = EEnemyState::Idle;

  /** Cached reference to the controlled enemy character. */
  UPROPERTY()
  TObjectPtr<ACharacterEnemy> EnemyCharacter;

  // --- Timers ---
  FTimerHandle StateTimerHandle;

  // --- Tuning ---

  /** How long the enemy idles before moving. */
  UPROPERTY(EditAnywhere, Category = "AI|Tuning")
  float IdleDuration = 2.5f;

  /** How close the enemy must get before starting attack warning. */
  UPROPERTY(EditAnywhere, Category = "AI|Tuning")
  float AttackRange = 400.f;

  /** How long the attack warning decal is shown. */
  UPROPERTY(EditAnywhere, Category = "AI|Tuning")
  float WarningDuration = 1.5f;

  // --- State transitions ---
  void EnterIdle();
  void EnterMoving();
  void EnterAttackWarning();

  /** Called when idle timer expires. */
  void OnIdleTimerExpired();

  /** Called when attack warning timer expires. */
  void OnWarningTimerExpired();

  /** Rotate to face the player character. */
  void FacePlayer(float DeltaTime);

  /** Get the player character in the world. */
  ACharacter *GetPlayerCharacter() const;
};
