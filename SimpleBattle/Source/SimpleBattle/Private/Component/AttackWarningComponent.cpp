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
}

void UAttackWarningComponent::BeginPlay() {
  Super::BeginPlay();
  CreateDecalComponent();
}

void UAttackWarningComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (!bIsWipeAnimating || !DynamicMaterial) {
    return;
  }

  WipeProgress += DeltaTime / WipeAnimDuration;
  if (WipeProgress >= 1.f) {
    WipeProgress = 1.f;
    bIsWipeAnimating = false;
  }

  DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), WipeProgress);
}

void UAttackWarningComponent::ShowWarning() {
  if (!DecalComp) {
    CreateDecalComponent();
  }
  if (!DecalComp) {
    return;
  }

  DecalComp->DecalSize =
      FVector(ProjectionDepth, WarningWidth * 0.5f, WarningLength * 0.5f);
  DecalComp->SetRelativeLocation(FVector(ForwardOffset, 0.f, 0.f));

  WipeProgress = 0.f;
  bIsWipeAnimating = true;
  if (DynamicMaterial) {
    DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);
  }

  DecalComp->SetVisibility(true);
}

void UAttackWarningComponent::HideWarning() {
  if (DecalComp) {
    DecalComp->SetVisibility(false);
  }
  bIsWipeAnimating = false;
  WipeProgress = 0.f;
  if (DynamicMaterial) {
    DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);
  }
}

void UAttackWarningComponent::CreateDecalComponent() {
  AActor *Owner = GetOwner();
  if (!Owner || DecalComp) {
    return;
  }

  USceneComponent *AttachParent = Owner->GetRootComponent();
  if (!AttachParent) {
    return;
  }

  DecalComp = NewObject<UDecalComponent>(
      Owner, UDecalComponent::StaticClass(),
      MakeUniqueObjectName(Owner, UDecalComponent::StaticClass(),
                           TEXT("AttackWarningDecal")));
  DecalComp->SetupAttachment(AttachParent);
  DecalComp->DecalSize =
      FVector(ProjectionDepth, WarningWidth * 0.5f, WarningLength * 0.5f);
  DecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
  DecalComp->SetRelativeLocation(FVector(ForwardOffset, 0.f, 0.f));
  DecalComp->SetVisibility(false);

  if (GetWorld() && GetWorld()->bIsWorldInitialized) {
    DecalComp->RegisterComponent();
  }

  if (DecalMaterialOverride) {
    DynamicMaterial = UMaterialInstanceDynamic::Create(DecalMaterialOverride, Owner);
  } else {
    CreateDefaultMaterial();
  }

  if (DynamicMaterial) {
    DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);
    DecalComp->SetDecalMaterial(DynamicMaterial);
  }
}

void UAttackWarningComponent::CreateDefaultMaterial() {
  AActor *Owner = GetOwner();
  if (!Owner) {
    return;
  }

  UMaterial *ParentMat = NewObject<UMaterial>(
      GetTransientPackage(), FName(TEXT("M_AttackWarning")), RF_Transient);
  ParentMat->MaterialDomain = MD_DeferredDecal;
  ParentMat->BlendMode = BLEND_Translucent;
  ParentMat->DecalBlendMode = DBM_Stain;

  // Expression graph wiring is editor-only. In packaged builds, provide a
  // proper asset through DecalMaterialOverride instead.
#if WITH_EDITOR
  UMaterialExpressionConstant3Vector *BaseColor =
      NewObject<UMaterialExpressionConstant3Vector>(ParentMat);
  BaseColor->Constant = FLinearColor(0.9f, 0.1f, 0.05f);

  UMaterialExpressionTextureCoordinate *TexCoord =
      NewObject<UMaterialExpressionTextureCoordinate>(ParentMat);

  UMaterialExpressionComponentMask *MaskU =
      NewObject<UMaterialExpressionComponentMask>(ParentMat);
  MaskU->R = true;
  MaskU->G = false;
  MaskU->B = false;
  MaskU->A = false;
  MaskU->Input.Connect(0, TexCoord);

  UMaterialExpressionScalarParameter *RevealProgressParam =
      NewObject<UMaterialExpressionScalarParameter>(ParentMat);
  RevealProgressParam->ParameterName = FName(TEXT("RevealProgress"));
  RevealProgressParam->DefaultValue = 0.f;

  UMaterialExpressionConstant *OpacityShown =
      NewObject<UMaterialExpressionConstant>(ParentMat);
  OpacityShown->R = 0.6f;

  UMaterialExpressionConstant *OpacityHidden =
      NewObject<UMaterialExpressionConstant>(ParentMat);
  OpacityHidden->R = 0.f;

  UMaterialExpressionIf *IfExpr = NewObject<UMaterialExpressionIf>(ParentMat);
  IfExpr->A.Connect(0, RevealProgressParam);
  IfExpr->B.Connect(0, MaskU);
  IfExpr->AGreaterThanB.Connect(0, OpacityShown);
  IfExpr->AEqualsB.Connect(0, OpacityShown);
  IfExpr->ALessThanB.Connect(0, OpacityHidden);

  ParentMat->GetExpressionCollection().AddExpression(BaseColor);
  ParentMat->GetExpressionCollection().AddExpression(TexCoord);
  ParentMat->GetExpressionCollection().AddExpression(MaskU);
  ParentMat->GetExpressionCollection().AddExpression(RevealProgressParam);
  ParentMat->GetExpressionCollection().AddExpression(OpacityShown);
  ParentMat->GetExpressionCollection().AddExpression(OpacityHidden);
  ParentMat->GetExpressionCollection().AddExpression(IfExpr);

  ParentMat->GetEditorOnlyData()->BaseColor.Connect(0, BaseColor);
  ParentMat->GetEditorOnlyData()->Opacity.Connect(0, IfExpr);
  ParentMat->PostEditChange();
#endif

  DynamicMaterial = UMaterialInstanceDynamic::Create(ParentMat, Owner);
  if (DynamicMaterial) {
    DynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);
  }
}

