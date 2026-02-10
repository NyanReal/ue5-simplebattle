// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterPlayer.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputDataAsset;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Top-down player character that looks toward the mouse cursor.
 * Input is configured via a UInputDataAsset instead of hardcoded references.
 */
UCLASS()
class SIMPLEBATTLE_API ACharacterPlayer : public ACharacter {
  GENERATED_BODY()

public:
  ACharacterPlayer();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;

private:
  // --- Components ---
  UPROPERTY(VisibleAnywhere, Category = "Camera")
  TObjectPtr<USpringArmComponent> SpringArmComp;

  UPROPERTY(VisibleAnywhere, Category = "Camera")
  TObjectPtr<UCameraComponent> CameraComp;

  // --- Input Data Asset ---
  UPROPERTY(EditDefaultsOnly, Category = "Input")
  TObjectPtr<UInputDataAsset> InputDataAsset;

  // --- Input Callbacks ---
  void Move(const FInputActionValue &Value);
  void Attack(const FInputActionValue &Value);

  // --- Cursor Rotation ---
  void RotateToCursor();
};
