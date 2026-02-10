// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TimerManager.h"
#include "EnemyAIController.generated.h"

class ACharacterEnemy;

UENUM()
enum class EEnemyState : uint8 {
  Idle,
  Moving,
  AttackWarning,
};

UCLASS()
class SIMPLEBATTLE_API AEnemyAIController : public AAIController {
  GENERATED_BODY()

public:
  AEnemyAIController();

protected:
  virtual void OnPossess(APawn *InPawn) override;
  virtual void Tick(float DeltaTime) override;

private:
  EEnemyState CurrentState = EEnemyState::Idle;

  UPROPERTY()
  TObjectPtr<ACharacterEnemy> EnemyCharacter;

  FTimerHandle StateTimerHandle;

  UPROPERTY(EditAnywhere, Category = "AI|Tuning")
  float IdleDuration = 2.5f;

  UPROPERTY(EditAnywhere, Category = "AI|Tuning")
  float AttackRange = 400.f;

  UPROPERTY(EditAnywhere, Category = "AI|Tuning")
  float WarningDuration = 1.5f;

  void EnterIdle();
  void EnterMoving();
  void EnterAttackWarning();

  void OnIdleTimerExpired();
  void OnWarningTimerExpired();

  void FacePlayer(float DeltaTime);
  ACharacter *GetPlayerCharacter() const;
};
