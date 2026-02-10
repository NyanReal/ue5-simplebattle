// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackWarningComponent.generated.h"

class UDecalComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIMPLEBATTLE_API UAttackWarningComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UAttackWarningComponent();

  UFUNCTION(BlueprintCallable, Category = "AttackWarning")
  void ShowWarning();

  UFUNCTION(BlueprintCallable, Category = "AttackWarning")
  void HideWarning();

protected:
  virtual void BeginPlay() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                             FActorComponentTickFunction *ThisTickFunction)
      override;

private:
  // --- Shape ---
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float WarningLength = 500.f;

  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float WarningWidth = 150.f;

  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float ForwardOffset = 250.f;

  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float ProjectionDepth = 200.f;

  // --- Animation ---
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Animation",
            meta = (ClampMin = "0.01"))
  float WipeAnimDuration = 0.4f;

  // --- Material ---
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Material")
  TObjectPtr<UMaterialInterface> DecalMaterialOverride;

  // --- Internal ---
  UPROPERTY()
  TObjectPtr<UDecalComponent> DecalComp;

  UPROPERTY()
  TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

  bool bIsWipeAnimating = false;
  float WipeProgress = 0.f;

  void CreateDecalComponent();
  void CreateDefaultMaterial();
};

