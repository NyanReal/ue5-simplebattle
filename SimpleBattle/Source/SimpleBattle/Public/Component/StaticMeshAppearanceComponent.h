// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "StaticMeshAppearanceComponent.generated.h"


class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Creates a chess-pawn-like appearance (cone body + sphere head) using
 * engine primitive meshes. Supports runtime dynamic material color changes.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIMPLEBATTLE_API UStaticMeshAppearanceComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UStaticMeshAppearanceComponent();

  /** Change the base color of the pawn at runtime. */
  UFUNCTION(BlueprintCallable, Category = "Appearance")
  void SetColor(FLinearColor NewColor);

protected:
  virtual void OnRegister() override;
  virtual void OnUnregister() override;

private:
  // --- Editable properties ---

  /** Base color of the pawn. */
  UPROPERTY(EditAnywhere, Category = "Appearance")
  FLinearColor BaseColor = FLinearColor(0.0f, 0.6f, 1.0f);

  /** Overall scale multiplier applied to the whole shape. */
  UPROPERTY(EditAnywhere, Category = "Appearance", meta = (ClampMin = "0.1"))
  float ShapeScale = 1.0f;

  // --- Internal components ---

  UPROPERTY()
  TObjectPtr<UStaticMeshComponent> BodyMesh;

  UPROPERTY()
  TObjectPtr<UStaticMeshComponent> HeadMesh;

  UPROPERTY()
  TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

  /** Build the cone + sphere shape and apply dynamic material. */
  void CreateShape();

  /** Destroy internal mesh components. */
  void DestroyShape();

  /** Create and apply a dynamic material instance to both meshes. */
  void ApplyDynamicMaterial();
};
