// Copyright SimpleBattle. All Rights Reserved.

#include "Character/CharacterEnemy.h"

#include "AI/EnemyAIController.h"
#include "Component/StaticMeshAppearanceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialInstanceDynamic.h"

ACharacterEnemy::ACharacterEnemy() {
  PrimaryActorTick.bCanEverTick = true;

  // Use code-driven AI controller
  AIControllerClass = AEnemyAIController::StaticClass();
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

  // Character movement settings for AI
  bUseControllerRotationYaw = false;
  GetCharacterMovement()->bOrientRotationToMovement = true;
  GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
  GetCharacterMovement()->MaxWalkSpeed = 300.f;

  // Appearance: reuse the chess-pawn component with red color
  AppearanceComp = CreateDefaultSubobject<UStaticMeshAppearanceComponent>(
      TEXT("Appearance"));

  // Warning decal: attached to root, initially hidden
  WarningDecalComp =
      CreateDefaultSubobject<UDecalComponent>(TEXT("WarningDecal"));
  WarningDecalComp->SetupAttachment(RootComponent);

  // Decal size: X = projection depth, Y = half-width, Z = half-length
  WarningDecalComp->DecalSize =
      FVector(200.f, WarningWidth * 0.5f, WarningLength * 0.5f);

  // Position: offset forward, rotate -90 pitch to project downward
  WarningDecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
  WarningDecalComp->SetRelativeLocation(
      FVector(WarningForwardOffset, 0.f, 0.f));

  WarningDecalComp->SetVisibility(false);
}

void ACharacterEnemy::BeginPlay() {
  Super::BeginPlay();

  // Set appearance to red
  if (AppearanceComp) {
    AppearanceComp->SetColor(FLinearColor(0.9f, 0.1f, 0.1f));
  }

  CreateDecalMaterial();
}

void ACharacterEnemy::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Animate the wipe reveal
  if (bIsWipeAnimating && DecalDynamicMaterial) {
    WipeProgress += DeltaTime / WipeAnimDuration;
    if (WipeProgress >= 1.f) {
      WipeProgress = 1.f;
      bIsWipeAnimating = false;
    }
    DecalDynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"),
                                                  WipeProgress);
  }
}

void ACharacterEnemy::ShowAttackWarning() {
  if (WarningDecalComp) {
    // Update decal size in case properties were edited
    WarningDecalComp->DecalSize =
        FVector(200.f, WarningWidth * 0.5f, WarningLength * 0.5f);
    WarningDecalComp->SetRelativeLocation(
        FVector(WarningForwardOffset, 0.f, 0.f));

    // Start wipe animation from 0
    WipeProgress = 0.f;
    bIsWipeAnimating = true;
    if (DecalDynamicMaterial) {
      DecalDynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"),
                                                    0.f);
    }

    WarningDecalComp->SetVisibility(true);
  }
}

void ACharacterEnemy::HideAttackWarning() {
  if (WarningDecalComp) {
    WarningDecalComp->SetVisibility(false);
    bIsWipeAnimating = false;
    WipeProgress = 0.f;
  }
}

void ACharacterEnemy::CreateDecalMaterial() {
  // Programmatically create a decal material with UV-based wipe reveal.
  // A "RevealProgress" parameter (0-1) controls how much of the rectangle
  // is visible, sweeping from near edge to far edge.
  UMaterial *ParentMat = NewObject<UMaterial>(
      GetTransientPackage(), FName(TEXT("M_WarningDecal")), RF_Transient);
  ParentMat->MaterialDomain = EMaterialDomain::MD_DeferredDecal;
  ParentMat->BlendMode = EBlendMode::BLEND_Translucent;
  ParentMat->DecalBlendMode = EDecalBlendMode::DBM_Stain;

#if WITH_EDITOR
  // --- Base color: solid red ---
  UMaterialExpressionConstant3Vector *ColorExpr =
      NewObject<UMaterialExpressionConstant3Vector>(ParentMat);
  ColorExpr->Constant = FLinearColor(0.9f, 0.1f, 0.05f);

  // --- Texture coordinate to get UV ---
  UMaterialExpressionTextureCoordinate *TexCoordExpr =
      NewObject<UMaterialExpressionTextureCoordinate>(ParentMat);
  TexCoordExpr->CoordinateIndex = 0;

  // --- Extract U channel (forward direction in decal space) ---
  UMaterialExpressionComponentMask *VMaskExpr =
      NewObject<UMaterialExpressionComponentMask>(ParentMat);
  VMaskExpr->R = true; // U channel = forward direction
  VMaskExpr->G = false;
  VMaskExpr->B = false;
  VMaskExpr->A = false;

  // --- RevealProgress scalar parameter (driven from C++ via MID) ---
  UMaterialExpressionScalarParameter *RevealParam =
      NewObject<UMaterialExpressionScalarParameter>(ParentMat);
  RevealParam->ParameterName = FName(TEXT("RevealProgress"));
  RevealParam->DefaultValue = 0.f;

  // --- If (RevealProgress > U) -> show, else -> hide ---
  // U=0 is near the character, U=1 is the far end.
  // As RevealProgress goes 0->1, it reveals from character outward.
  UMaterialExpressionIf *IfExpr = NewObject<UMaterialExpressionIf>(ParentMat);

  // Visible opacity value
  UMaterialExpressionConstant *VisibleOpacity =
      NewObject<UMaterialExpressionConstant>(ParentMat);
  VisibleOpacity->R = 0.6f;

  // Hidden opacity value
  UMaterialExpressionConstant *HiddenOpacity =
      NewObject<UMaterialExpressionConstant>(ParentMat);
  HiddenOpacity->R = 0.f;

  // Register all expressions
  ParentMat->GetExpressionCollection().AddExpression(ColorExpr);
  ParentMat->GetExpressionCollection().AddExpression(TexCoordExpr);
  ParentMat->GetExpressionCollection().AddExpression(VMaskExpr);
  ParentMat->GetExpressionCollection().AddExpression(RevealParam);
  ParentMat->GetExpressionCollection().AddExpression(IfExpr);
  ParentMat->GetExpressionCollection().AddExpression(VisibleOpacity);
  ParentMat->GetExpressionCollection().AddExpression(HiddenOpacity);

  // Wire connections:
  // TexCoord -> ComponentMask(U) -> If.B (directly, no inversion)
  VMaskExpr->Input.Connect(0, TexCoordExpr);

  // If.A = RevealProgress, If.B = U
  // If A > B: show (pixel is within revealed area)
  // If A < B: hide
  IfExpr->A.Connect(0, RevealParam);
  IfExpr->B.Connect(0, VMaskExpr);
  IfExpr->AGreaterThanB.Connect(0, VisibleOpacity);
  IfExpr->AEqualsB.Connect(0, VisibleOpacity);
  IfExpr->ALessThanB.Connect(0, HiddenOpacity);

  // Final wiring: Color -> BaseColor, If result -> Opacity
  ParentMat->GetEditorOnlyData()->BaseColor.Connect(0, ColorExpr);
  ParentMat->GetEditorOnlyData()->Opacity.Connect(0, IfExpr);

  ParentMat->PostEditChange();
#endif

  DecalDynamicMaterial = UMaterialInstanceDynamic::Create(ParentMat, this);
  // Initialize reveal to 0 (hidden)
  DecalDynamicMaterial->SetScalarParameterValue(TEXT("RevealProgress"), 0.f);

  if (WarningDecalComp) {
    WarningDecalComp->SetDecalMaterial(DecalDynamicMaterial);
  }
}
