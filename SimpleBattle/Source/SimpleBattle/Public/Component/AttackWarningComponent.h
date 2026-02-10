// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "AttackWarningComponent.generated.h"


class UDecalComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

/**
 * Manages a rectangular decal that animates as a forward-sweeping wipe
 * to warn players about an incoming attack zone.
 *
 * Attach this component to any character/actor that needs an AoE warning.
 * Call ShowWarning() to start the wipe animation and HideWarning() to dismiss.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIMPLEBATTLE_API UAttackWarningComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UAttackWarningComponent();

  /** Begin the forward-sweep wipe animation and show the decal. */
  UFUNCTION(BlueprintCallable, Category = "AttackWarning")
  void ShowWarning();

  /** Hide the decal and stop any animation. */
  UFUNCTION(BlueprintCallable, Category = "AttackWarning")
  void HideWarning();

  /** Returns true while the wipe animation is actively playing. */
  UFUNCTION(BlueprintCallable, Category = "AttackWarning")
  bool IsAnimating() const { return bIsWipeAnimating; }

protected:
  virtual void BeginPlay() override;
  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;

private:
  // --- Decal settings (editable) ---

  /** Length of the warning rectangle (forward direction). */
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float WarningLength = 500.f;

  /** Width of the warning rectangle. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float WarningWidth = 150.f;

  /** Forward offset from the owner's center. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float ForwardOffset = 250.f;

  /** Projection depth (how far above/below the decal projects). */
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Shape")
  float ProjectionDepth = 200.f;

  /** Duration of the wipe reveal animation in seconds. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Animation")
  float WipeAnimDuration = 0.4f;

  /** Optional material override. If null, a default red decal is created. */
  UPROPERTY(EditAnywhere, Category = "AttackWarning|Material")
  TObjectPtr<UMaterialInterface> DecalMaterialOverride;

  // --- Internal ---
  UPROPERTY()
  TObjectPtr<UDecalComponent> DecalComp;

  UPROPERTY()
  TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

  bool bIsWipeAnimating = false;
  float WipeProgress = 0.f;

  /** Create the decal component and attach to owner. */
  void CreateDecalComponent();

  /** Create a default red wipe decal material programmatically. */
  void CreateDefaultMaterial();
};
