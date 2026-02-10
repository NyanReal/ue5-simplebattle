// Copyright SimpleBattle. All Rights Reserved.

#include "AI/EnemyAIController.h"

#include "Character/CharacterEnemy.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

AEnemyAIController::AEnemyAIController() {
  PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn *InPawn) {
  Super::OnPossess(InPawn);

  EnemyCharacter = Cast<ACharacterEnemy>(InPawn);
  if (!EnemyCharacter) {
    UE_LOG(LogTemp, Warning,
           TEXT("EnemyAIController: Possessed pawn is not ACharacterEnemy."));
    return;
  }

  EnterIdle();
}

void AEnemyAIController::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (!EnemyCharacter) {
    return;
  }

  switch (CurrentState) {
  case EEnemyState::Moving: {
    // Check if close enough to the player to start attack warning
    ACharacter *Player = GetPlayerCharacter();
    if (Player) {
      const float Distance = FVector::Dist(EnemyCharacter->GetActorLocation(),
                                           Player->GetActorLocation());
      if (Distance <= AttackRange) {
        // Stop movement and enter attack warning
        StopMovement();
        EnterAttackWarning();
      }
    }
    break;
  }
  case EEnemyState::AttackWarning:
    // Continuously face the player during warning phase
    FacePlayer(DeltaTime);
    break;
  default:
    break;
  }
}

void AEnemyAIController::EnterIdle() {
  CurrentState = EEnemyState::Idle;

  UE_LOG(LogTemp, Log, TEXT("Enemy AI: Idle"));

  // Start idle timer
  GetWorld()->GetTimerManager().SetTimer(
      StateTimerHandle, this, &AEnemyAIController::OnIdleTimerExpired,
      IdleDuration, false);
}

void AEnemyAIController::EnterMoving() {
  CurrentState = EEnemyState::Moving;

  UE_LOG(LogTemp, Log, TEXT("Enemy AI: Moving toward player"));

  // Move toward the player
  ACharacter *Player = GetPlayerCharacter();
  if (Player) {
    MoveToActor(Player, AttackRange * 0.8f);
  } else {
    // No player found, go back to idle
    EnterIdle();
  }
}

void AEnemyAIController::EnterAttackWarning() {
  CurrentState = EEnemyState::AttackWarning;

  UE_LOG(LogTemp, Log, TEXT("Enemy AI: Attack Warning!"));

  // Disable movement rotation so we control facing manually
  if (EnemyCharacter) {
    EnemyCharacter->ShowAttackWarning();
  }

  // Start warning timer
  GetWorld()->GetTimerManager().SetTimer(
      StateTimerHandle, this, &AEnemyAIController::OnWarningTimerExpired,
      WarningDuration, false);
}

void AEnemyAIController::OnIdleTimerExpired() { EnterMoving(); }

void AEnemyAIController::OnWarningTimerExpired() {
  if (EnemyCharacter) {
    EnemyCharacter->HideAttackWarning();
  }

  UE_LOG(LogTemp, Log, TEXT("Enemy AI: Attack finished, returning to Idle"));
  EnterIdle();
}

void AEnemyAIController::FacePlayer(float DeltaTime) {
  ACharacter *Player = GetPlayerCharacter();
  if (!Player || !EnemyCharacter) {
    return;
  }

  FVector Direction =
      Player->GetActorLocation() - EnemyCharacter->GetActorLocation();
  Direction.Z = 0.f;

  if (!Direction.IsNearlyZero()) {
    const FRotator TargetRotation = Direction.Rotation();
    const FRotator CurrentRotation = EnemyCharacter->GetActorRotation();
    const FRotator NewRotation =
        FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 10.f);
    EnemyCharacter->SetActorRotation(NewRotation);
  }
}

ACharacter *AEnemyAIController::GetPlayerCharacter() const {
  return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}
