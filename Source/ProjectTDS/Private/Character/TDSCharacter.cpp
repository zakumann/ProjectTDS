// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TDSCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

// Sets default values
ATDSCharacter::ATDSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
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
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FVector ForwardDirection(1.0f, 0.0f, 0.0f);
		const FVector RightDirection(0.0f, 1.0f, 0.0f);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATDSCharacter::RotateToMouseCursor()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		FHitResult TraceHitResult;

		if (PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, TraceHitResult))
		{
			FVector MouseWorldLocation = TraceHitResult.ImpactPoint;

			FVector TargetDirection = MouseWorldLocation - GetActorLocation();

			TargetDirection.Z = 0.0f;

			if (!TargetDirection.IsNearlyZero())
			{
				FRotator TargetRotation = TargetDirection.Rotation();

				FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 15.0f);
				SetActorRotation(NewRotation);
			}
		}
	}
}
