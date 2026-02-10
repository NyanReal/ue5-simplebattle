// Copyright SimpleBattle. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputDataAsset.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * Data asset that bundles Input Actions and Input Mapping Context together.
 * Assign this to the player character instead of hardcoding individual IA/IMC references.
 */
UCLASS(BlueprintType)
class SIMPLEBATTLE_API UInputDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
};
