// Copyright SimpleBattle. All Rights Reserved.

#include "Component/AttackWarningComponent.h"

#include "Components/DecalComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialInstanceDynamic.h"

UAttackWarningComponent::UAttackWarningComponent() {
  PrimaryComponentTick.bCanEverTick = true;
  // Tick is enabled but only does work when animating
}

void UAttackWarningComponent::BeginPlay() {
  Super::BeginPlay();
  CreateDecalComponent();
}

void UAttackWarningComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Animate the wipe reveal
  if (bIsWipeAnimating && DynamicMaterial) {
    WipeProgress += DeltaTime / WipeAnimDuration;
    if (WipeProgress >= 1.f) {
      WipeProgress = 1.f;
      bIsWipeAnimating = false;
    }
    DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"),
                                             WipeProgress);
  }
}

void UAttackWarningComponent::ShowWarning() {
  if (DecalComp) {
    // Update shape in case properties were changed at runtime
    DecalComp->DecalSize =
        FVector(ProjectionDepth, WarningWidth * 0.5f, WarningLength * 0.5f);
    DecalComp->SetRelativeLocation(FVector(ForwardOffset, 0.f, 0.f));

    // Start wipe from 0
    WipeProgress = 0.f;
    bIsWipeAnimating = true;
    if (DynamicMaterial) {
      DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);
    }

    DecalComp->SetVisibility(true);
  }
}

void UAttackWarningComponent::HideWarning() {
  if (DecalComp) {
    DecalComp->SetVisibility(false);
    bIsWipeAnimating = false;
    WipeProgress = 0.f;
  }
}

void UAttackWarningComponent::CreateDecalComponent() {
  AActor *Owner = GetOwner();
  if (!Owner) {
    return;
  }

  // Create and attach the decal component at runtime
  DecalComp = NewObject<UDecalComponent>(
      Owner, UDecalComponent::StaticClass(),
      MakeUniqueObjectName(Owner, UDecalComponent::StaticClass(),
                           TEXT("AttackWarningDecal")));
  DecalComp->SetupAttachment(Owner->GetRootComponent());
  DecalComp->DecalSize =
      FVector(ProjectionDepth, WarningWidth * 0.5f, WarningLength * 0.5f);
  DecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
  DecalComp->SetRelativeLocation(FVector(ForwardOffset, 0.f, 0.f));
  DecalComp->SetVisibility(false);

  if (GetWorld() && GetWorld()->bIsWorldInitialized) {
    DecalComp->RegisterComponent();
  }

  // Use material override if provided, otherwise create a default one
  if (DecalMaterialOverride) {
    DynamicMaterial =
        UMaterialInstanceDynamic::Create(DecalMaterialOverride, Owner);
  } else {
    CreateDefaultMaterial();
  }

  if (DecalComp && DynamicMaterial) {
    DecalComp->SetDecalMaterial(DynamicMaterial);
  }
}

void UAttackWarningComponent::CreateDefaultMaterial() {
  AActor *Owner = GetOwner();

  // Programmatically create a Stain decal material with UV-based wipe reveal.
  UMaterial *ParentMat = NewObject<UMaterial>(
      GetTransientPackage(), FName(TEXT("M_AttackWarning")), RF_Transient);
  ParentMat->MaterialDomain = EMaterialDomain::MD_DeferredDecal;
  ParentMat->BlendMode = EBlendMode::BLEND_Translucent;
  ParentMat->DecalBlendMode = EDecalBlendMode::DBM_Stain;

#if WITH_EDITOR
  // Base color: red warning zone
  UMaterialExpressionConstant3Vector *ColorExpr =
      NewObject<UMaterialExpressionConstant3Vector>(ParentMat);
  ColorExpr->Constant = FLinearColor(0.9f, 0.1f, 0.05f);

  // Texture coordinate for UV access
  UMaterialExpressionTextureCoordinate *TexCoordExpr =
      NewObject<UMaterialExpressionTextureCoordinate>(ParentMat);
  TexCoordExpr->CoordinateIndex = 0;

  // Extract U channel (forward direction in decal space)
  UMaterialExpressionComponentMask *UMaskExpr =
      NewObject<UMaterialExpressionComponentMask>(ParentMat);
  UMaskExpr->R = true; // U channel = forward direction
  UMaskExpr->G = false;
  UMaskExpr->B = false;
  UMaskExpr->A = false;

  // RevealProgress parameter (animated from C++)
  UMaterialExpressionScalarParameter *RevealParam =
      NewObject<UMaterialExpressionScalarParameter>(ParentMat);
  RevealParam->ParameterName = FName(TEXT("RevealProgress"));
  RevealParam->DefaultValue = 0.f;

  // If (RevealProgress > U) -> visible, else -> hidden
  UMaterialExpressionIf *IfExpr = NewObject<UMaterialExpressionIf>(ParentMat);

  UMaterialExpressionConstant *VisibleOpacity =
      NewObject<UMaterialExpressionConstant>(ParentMat);
  VisibleOpacity->R = 0.6f;

  UMaterialExpressionConstant *HiddenOpacity =
      NewObject<UMaterialExpressionConstant>(ParentMat);
  HiddenOpacity->R = 0.f;

  // Register expressions
  ParentMat->GetExpressionCollection().AddExpression(ColorExpr);
  ParentMat->GetExpressionCollection().AddExpression(TexCoordExpr);
  ParentMat->GetExpressionCollection().AddExpression(UMaskExpr);
  ParentMat->GetExpressionCollection().AddExpression(RevealParam);
  ParentMat->GetExpressionCollection().AddExpression(IfExpr);
  ParentMat->GetExpressionCollection().AddExpression(VisibleOpacity);
  ParentMat->GetExpressionCollection().AddExpression(HiddenOpacity);

  // Wire: TexCoord -> ComponentMask(U) -> If.B
  UMaskExpr->Input.Connect(0, TexCoordExpr);

  // If.A = RevealProgress, If.B = U
  IfExpr->A.Connect(0, RevealParam);
  IfExpr->B.Connect(0, UMaskExpr);
  IfExpr->AGreaterThanB.Connect(0, VisibleOpacity);
  IfExpr->AEqualsB.Connect(0, VisibleOpacity);
  IfExpr->ALessThanB.Connect(0, HiddenOpacity);

  // Output: Color -> BaseColor, If -> Opacity
  ParentMat->GetEditorOnlyData()->BaseColor.Connect(0, ColorExpr);
  ParentMat->GetEditorOnlyData()->Opacity.Connect(0, IfExpr);

  ParentMat->PostEditChange();
#endif

  DynamicMaterial = UMaterialInstanceDynamic::Create(ParentMat, Owner);
  DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);
}
