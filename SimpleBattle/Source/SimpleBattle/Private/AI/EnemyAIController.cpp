// Copyright SimpleBattle. All Rights Reserved.

#include "AI/EnemyAIController.h"

#include "Character/CharacterEnemy.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController() {
  PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn *InPawn) {
  Super::OnPossess(InPawn);

  EnemyCharacter = Cast<ACharacterEnemy>(InPawn);
  if (!EnemyCharacter) {
    UE_LOG(LogTemp, Warning, TEXT("EnemyAIController: Failed to possess %s"),
           InPawn ? *InPawn->GetName() : TEXT("<null>"));
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
    ACharacter *Player = GetPlayerCharacter();
    if (!Player) {
      return;
    }

    const float Distance = FVector::Dist(EnemyCharacter->GetActorLocation(),
                                         Player->GetActorLocation());
    if (Distance <= AttackRange) {
      StopMovement();
      EnterAttackWarning();
    }
  } break;

  case EEnemyState::AttackWarning:
    FacePlayer(DeltaTime);
    break;

  case EEnemyState::Idle:
  default:
    break;
  }
}

void AEnemyAIController::EnterIdle() {
  GetWorldTimerManager().ClearTimer(StateTimerHandle);
  CurrentState = EEnemyState::Idle;

  GetWorldTimerManager().SetTimer(
      StateTimerHandle, this, &AEnemyAIController::OnIdleTimerExpired,
      IdleDuration, false);
}

void AEnemyAIController::OnIdleTimerExpired() { EnterMoving(); }

void AEnemyAIController::EnterMoving() {
  GetWorldTimerManager().ClearTimer(StateTimerHandle);
  CurrentState = EEnemyState::Moving;

  ACharacter *Player = GetPlayerCharacter();
  if (!Player) {
    EnterIdle();
    return;
  }

  MoveToActor(Player, AttackRange * 0.8f);
}

void AEnemyAIController::EnterAttackWarning() {
  GetWorldTimerManager().ClearTimer(StateTimerHandle);
  CurrentState = EEnemyState::AttackWarning;

  if (EnemyCharacter) {
    EnemyCharacter->ShowAttackWarning();
  }

  GetWorldTimerManager().SetTimer(
      StateTimerHandle, this, &AEnemyAIController::OnWarningTimerExpired,
      WarningDuration, false);
}

void AEnemyAIController::OnWarningTimerExpired() {
  if (EnemyCharacter) {
    EnemyCharacter->HideAttackWarning();
  }
  EnterIdle();
}

void AEnemyAIController::FacePlayer(float DeltaTime) {
  if (!EnemyCharacter) {
    return;
  }

  ACharacter *Player = GetPlayerCharacter();
  if (!Player) {
    return;
  }

  FVector Direction =
      Player->GetActorLocation() - EnemyCharacter->GetActorLocation();
  Direction.Z = 0.f;
  if (Direction.IsNearlyZero()) {
    return;
  }

  const FRotator TargetRotation = Direction.Rotation();
  const FRotator NewRotation = FMath::RInterpTo(
      EnemyCharacter->GetActorRotation(), TargetRotation, DeltaTime, 10.f);
  EnemyCharacter->SetActorRotation(NewRotation);
}

ACharacter *AEnemyAIController::GetPlayerCharacter() const {
  return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

