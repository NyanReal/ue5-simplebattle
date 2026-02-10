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

  // Spawn one test enemy in front of the default player start
  UWorld *World = GetWorld();
  if (World) {
    FVector SpawnLocation(800.f, 0.f, 100.f);
    FRotator SpawnRotation(0.f, 180.f, 0.f); // Facing back toward origin
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ACharacterEnemy *Enemy = World->SpawnActor<ACharacterEnemy>(
        ACharacterEnemy::StaticClass(), SpawnLocation, SpawnRotation,
        SpawnParams);
    if (Enemy) {
      UE_LOG(LogTemp, Log, TEXT("SimpleGameMode: Spawned test enemy at %s"),
             *SpawnLocation.ToString());
    }
  }
}
