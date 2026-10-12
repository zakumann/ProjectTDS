// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TDSCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"

// Sets default values
ATDSCharacter::ATDSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 1200.f;
	SpringArmComp->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritRoll = false;
	SpringArmComp->bInheritYaw = false;

	TopDownCameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCameraComp"));
	TopDownCameraComp->SetupAttachment(SpringArmComp);
	TopDownCameraComp->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void ATDSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (InputMappingContext.IsValid())
			{
				Subsystem->AddMappingContext(InputMappingContext.Get(), 0);
			}
		}

		PlayerController->bShowMouseCursor = true;
	}
}

// Called every frame
void ATDSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateToMouseCursor();
}

// Called to bind functionality to input
void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDSCharacter::Move);
		}
	}
}


void ATDSCharacter::Move(const FInputActionValue& Value)
{
	if (!TopDownCameraComp) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (MovementVector.IsNearlyZero())return;

	FVector Forward = TopDownCameraComp->GetForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	FVector Right = TopDownCameraComp->GetRightVector();
	Right.Z = 0.f;
	Right.Normalize();

	AddMovementInput(Forward, MovementVector.Y);
	AddMovementInput(Right, MovementVector.X);
}

void ATDSCharacter::RotateToMouseCursor()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)return;

	FVector WorldLocation;
	FVector WorldDirection;
	if (!PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return;
	}

	if (FMath::IsNearlyZero(WorldDirection.Z, KINDA_SMALL_NUMBER))
	{
		return;
	}

	const FVector ActorLocation = GetActorLocation();
	const float Distance = (ActorLocation.Z - WorldLocation.Z) / WorldDirection.Z;
	if (Distance < 0.f)
	{
		return;
	}

	const FVector IntersectionPoint = WorldLocation + WorldDirection * Distance;

	FVector TargetDirection = IntersectionPoint - ActorLocation;
	TargetDirection.Z = 0.f;

	if (TargetDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRotation = TargetDirection.Rotation();
	const FRotator NewRotation = FMath::RInterpTo(
		GetActorRotation(),
		TargetRotation,
		GetWorld()->GetDeltaSeconds(),
		RotationInterpSpeed);

	SetActorRotation(NewRotation);
}
