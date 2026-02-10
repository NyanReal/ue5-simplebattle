// Copyright SimpleBattle. All Rights Reserved.

#include "Game/SimpleGameMode.h"

#include "Character/CharacterEnemy.h"
#include "Character/CharacterPlayer.h"
#include "Engine/World.h"
#include "Player/SimplePlayerController.h"

ASimpleGameMode::ASimpleGameMode() {
  DefaultPawnClass = ACharacterPlayer::StaticClass();
  PlayerControllerClass = ASimplePlayerController::StaticClass();
}

void ASimpleGameMode::BeginPlay() {
  Super::BeginPlay();

  UWorld *World = GetWorld();
  if (!World) {
    return;
  }

  const FVector SpawnLocation(800.f, 0.f, 100.f);
  const FRotator SpawnRotation(0.f, 180.f, 0.f);

  FActorSpawnParameters Params;
  Params.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

  ACharacterEnemy *Enemy =
      World->SpawnActor<ACharacterEnemy>(ACharacterEnemy::StaticClass(),
                                         SpawnLocation, SpawnRotation, Params);
  if (!Enemy) {
    UE_LOG(LogTemp, Warning, TEXT("SimpleGameMode: Failed to spawn enemy."));
  }
}
