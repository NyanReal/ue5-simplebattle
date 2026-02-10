// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterEnemy.generated.h"

class UStaticMeshAppearanceComponent;
class UAttackWarningComponent;

UCLASS()
class SIMPLEBATTLE_API ACharacterEnemy : public ACharacter {
  GENERATED_BODY()

public:
  ACharacterEnemy();

  void ShowAttackWarning();
  void HideAttackWarning();

protected:
  virtual void BeginPlay() override;

private:
  UPROPERTY(VisibleAnywhere, Category = "Appearance")
  TObjectPtr<UStaticMeshAppearanceComponent> AppearanceComp;

  UPROPERTY(VisibleAnywhere, Category = "AttackWarning")
  TObjectPtr<UAttackWarningComponent> AttackWarningComp;
};

