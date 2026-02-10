// Copyright SimpleBattle. All Rights Reserved.

#include "Game/SimpleGameMode.h"
#include "Character/CharacterPlayer.h"
#include "Player/SimplePlayerController.h"

ASimpleGameMode::ASimpleGameMode() {
  DefaultPawnClass = ACharacterPlayer::StaticClass();
  PlayerControllerClass = ASimplePlayerController::StaticClass();
}
