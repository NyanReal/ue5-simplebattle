// Copyright SimpleBattle. All Rights Reserved.

#include "Player/SimplePlayerController.h"

void ASimplePlayerController::BeginPlay() {
  Super::BeginPlay();

  // Show mouse cursor for top-down gameplay
  bShowMouseCursor = true;

  // Set input mode to allow both game and UI input
  FInputModeGameAndUI InputMode;
  InputMode.SetHideCursorDuringCapture(false);
  SetInputMode(InputMode);
}
