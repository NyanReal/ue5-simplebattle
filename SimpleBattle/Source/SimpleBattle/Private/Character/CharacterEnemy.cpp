// Copyright SimpleBattle. All Rights Reserved.

#include "Character/CharacterEnemy.h"

#include "AI/EnemyAIController.h"
#include "Component/AttackWarningComponent.h"
#include "Component/StaticMeshAppearanceComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACharacterEnemy::ACharacterEnemy() {
  PrimaryActorTick.bCanEverTick = false;

  // Use code-driven AI controller
  AIControllerClass = AEnemyAIController::StaticClass();
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

  // Character movement settings for AI
  bUseControllerRotationYaw = false;
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
  GetCharacterMovement()->MaxWalkSpeed = 300.f;

  // Appearance: chess-pawn shape with red color
  AppearanceComp = CreateDefaultSubobject<UStaticMeshAppearanceComponent>(
      TEXT("Appearance"));

  // Attack warning: forward rectangular decal with wipe animation
  AttackWarningComp =
      CreateDefaultSubobject<UAttackWarningComponent>(TEXT("AttackWarning"));
}

void ACharacterEnemy::BeginPlay() {
  Super::BeginPlay();

  // Set appearance to red
  if (AppearanceComp) {
    AppearanceComp->SetColor(FLinearColor(0.9f, 0.1f, 0.1f));
  }
}

void ACharacterEnemy::ShowAttackWarning() {
  if (AttackWarningComp) {
    AttackWarningComp->ShowWarning();
  }
}

void ACharacterEnemy::HideAttackWarning() {
  if (AttackWarningComp) {
    AttackWarningComp->HideWarning();
  }
}
