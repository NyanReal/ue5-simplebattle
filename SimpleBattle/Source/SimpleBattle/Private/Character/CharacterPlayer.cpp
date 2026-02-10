// Copyright SimpleBattle. All Rights Reserved.

#include "Character/CharacterPlayer.h"
#include "Input/InputDataAsset.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"


ACharacterPlayer::ACharacterPlayer() {
  PrimaryActorTick.bCanEverTick = true;

  // Decouple character rotation from controller rotation
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = false;
  bUseControllerRotationRoll = false;

  // Do not orient rotation to movement direction (cursor handles rotation)
  GetCharacterMovement()->bOrientRotationToMovement = false;
  GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

  // Spring arm for top-down camera
  SpringArmComp =
      CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
  SpringArmComp->SetupAttachment(RootComponent);
  SpringArmComp->TargetArmLength = 1500.f;
  SpringArmComp->SetRelativeRotation(
      FRotator(-50.f, 0.f, 0.f)); // Top-down angle, +X forward
  SpringArmComp->bUsePawnControlRotation =
      false; // Fixed camera, no controller rotation
  SpringArmComp->bDoCollisionTest = false;
  SpringArmComp->bInheritPitch = false;
  SpringArmComp->bInheritYaw = false;
  SpringArmComp->bInheritRoll = false;

  // Camera
  CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
  CameraComp->bUsePawnControlRotation = false;
}

void ACharacterPlayer::BeginPlay() {
  Super::BeginPlay();

  // Add Input Mapping Context from the Data Asset
  if (InputDataAsset) {
    if (const APlayerController *PC =
            Cast<APlayerController>(GetController())) {
      if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
              ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                  PC->GetLocalPlayer())) {
        Subsystem->AddMappingContext(InputDataAsset->DefaultMappingContext, 0);
      }
    }
  }
}

void ACharacterPlayer::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  RotateToCursor();
}

void ACharacterPlayer::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (!InputDataAsset) {
    UE_LOG(LogTemp, Error, TEXT("InputDataAsset is not assigned on %s"),
           *GetName());
    return;
  }

  UEnhancedInputComponent *EnhancedInput =
      CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

  // Bind Move (triggered every frame while held)
  EnhancedInput->BindAction(InputDataAsset->MoveAction,
                            ETriggerEvent::Triggered, this,
                            &ACharacterPlayer::Move);

  // Bind Attack (started on press)
  EnhancedInput->BindAction(InputDataAsset->AttackAction,
                            ETriggerEvent::Started, this,
                            &ACharacterPlayer::Attack);

  // Bind Jump (use built-in ACharacter jump)
  EnhancedInput->BindAction(InputDataAsset->JumpAction, ETriggerEvent::Started,
                            this, &ACharacter::Jump);
  EnhancedInput->BindAction(InputDataAsset->JumpAction,
                            ETriggerEvent::Completed, this,
                            &ACharacter::StopJumping);
}

void ACharacterPlayer::Move(const FInputActionValue &Value) {
  const FVector2D MoveInput = Value.Get<FVector2D>();

  // World-space movement relative to fixed camera (+X forward)
  const FVector ForwardDir = FVector::ForwardVector; // +X
  const FVector RightDir = FVector::RightVector;     // +Y

  AddMovementInput(ForwardDir, MoveInput.Y);
  AddMovementInput(RightDir, MoveInput.X);
}

void ACharacterPlayer::Attack(const FInputActionValue &Value) {
  UE_LOG(LogTemp, Log, TEXT("Attack action triggered!"));
  // TODO: Implement attack logic
}

void ACharacterPlayer::RotateToCursor() {
  const APlayerController *PC = Cast<APlayerController>(GetController());
  if (!PC) {
    return;
  }

  FHitResult HitResult;
  if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult)) {
    const FVector CursorWorldPos = HitResult.ImpactPoint;
    const FVector ActorLocation = GetActorLocation();

    FVector Direction = CursorWorldPos - ActorLocation;
    Direction.Z = 0.f; // Ignore height difference

    if (!Direction.IsNearlyZero()) {
      const FRotator TargetRotation = Direction.Rotation();
      SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation,
                                        GetWorld()->GetDeltaSeconds(), 15.f));
    }
  }
}
