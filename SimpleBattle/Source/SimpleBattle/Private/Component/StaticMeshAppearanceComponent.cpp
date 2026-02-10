// Copyright SimpleBattle. All Rights Reserved.

#include "Component/StaticMeshAppearanceComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UStaticMeshAppearanceComponent::UStaticMeshAppearanceComponent() {
  PrimaryComponentTick.bCanEverTick = false;
}

void UStaticMeshAppearanceComponent::OnRegister() {
  Super::OnRegister();
  CreateShape();
}

void UStaticMeshAppearanceComponent::OnUnregister() {
  DestroyShape();
  Super::OnUnregister();
}

void UStaticMeshAppearanceComponent::SetColor(FLinearColor NewColor) {
  BaseColor = NewColor;
  if (DynamicMaterial) {
    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), NewColor);
  }
}

void UStaticMeshAppearanceComponent::CreateShape() {
  AActor *Owner = GetOwner();
  if (!Owner || BodyMesh) {
    return;
  }

  // Find engine primitive meshes
  UStaticMesh *ConeMesh =
      LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
  UStaticMesh *SphereMesh = LoadObject<UStaticMesh>(
      nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));

  if (!ConeMesh || !SphereMesh) {
    UE_LOG(LogTemp, Warning,
           TEXT("StaticMeshAppearanceComponent: Failed to load basic shapes."));
    return;
  }

  // --- Body (Cone) ---
  BodyMesh = NewObject<UStaticMeshComponent>(
      Owner, UStaticMeshComponent::StaticClass(),
      MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(),
                           TEXT("PawnBody")));
  BodyMesh->SetupAttachment(Owner->GetRootComponent());
  BodyMesh->SetStaticMesh(ConeMesh);
  // Scale cone: taller and narrower to look like a chess pawn body
  BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.2f) * ShapeScale);
  BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
  BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // --- Head (Sphere) ---
  HeadMesh = NewObject<UStaticMeshComponent>(
      Owner, UStaticMeshComponent::StaticClass(),
      MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(),
                           TEXT("PawnHead")));
  HeadMesh->SetupAttachment(BodyMesh);
  HeadMesh->SetStaticMesh(SphereMesh);
  // Smaller sphere sitting on top of the cone
  HeadMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
  // Position on top of scaled cone (cone height ~100 * 1.2 scale = 120)
  HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 95.f));
  HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // Register components if world is ready
  if (GetWorld() && GetWorld()->bIsWorldInitialized) {
    BodyMesh->RegisterComponent();
    HeadMesh->RegisterComponent();
  }

  ApplyDynamicMaterial();
}

void UStaticMeshAppearanceComponent::DestroyShape() {
  if (HeadMesh) {
    HeadMesh->DestroyComponent();
    HeadMesh = nullptr;
  }
  if (BodyMesh) {
    BodyMesh->DestroyComponent();
    BodyMesh = nullptr;
  }
  DynamicMaterial = nullptr;
}

void UStaticMeshAppearanceComponent::ApplyDynamicMaterial() {
  if (!BodyMesh) {
    return;
  }

  // Create dynamic material from engine default
  UMaterialInterface *BaseMat = LoadObject<UMaterialInterface>(
      nullptr,
      TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

  if (!BaseMat) {
    UE_LOG(LogTemp, Warning,
           TEXT("StaticMeshAppearanceComponent: Failed to load base "
                "material."));
    return;
  }

  DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMat, GetOwner());
  DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);

  BodyMesh->SetMaterial(0, DynamicMaterial);
  if (HeadMesh) {
    HeadMesh->SetMaterial(0, DynamicMaterial);
  }
}
