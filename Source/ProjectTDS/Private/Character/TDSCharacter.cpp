// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TDSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATDSCharacter::ATDSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 1200.f;
	SpringArmComp->SetRelativeRotation(FRotator(-50.f, -45.f, 0.f));
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritYaw = false;
	SpringArmComp->bInheritRoll = false;
	
	TopDownCameraComp = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComp");
	TopDownCameraComp->SetupAttachment(SpringArmComp);
	TopDownCameraComp->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
}

// Called when the game starts or when spawned
void ATDSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDSCharacter::Move);
	}
}

void ATDSCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		/*const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);*/

		const float CameraYaw = SpringArmComp->GetRelativeRotation().Yaw;
		const FRotator IsometricRotation(0.f, CameraYaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(IsometricRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(IsometricRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATDSCharacter::RotateToMouseCursor()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FVector MouseWorldLocation = GetMouseCursorWorldLocation();
		FVector CharacterLocation = GetActorLocation();

		// Looking forward related with Z side.
		MouseWorldLocation.Z = CharacterLocation.Z;

		// Calcuate Looking direction
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, MouseWorldLocation);

		// Affect only Yaw
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw = LookAtRotation.Yaw;

		SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 15.f));
	}
}

FVector ATDSCharacter::GetMouseCursorWorldLocation() const
{
	FVector WorldLocation, WorldDirection;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			// Calculate PalneOrigin with Z=0 / height of character
			FVector PlaneOrigin = GetActorLocation();
			PlaneOrigin.Z = 0.f;

			float T;
			FVector Intersection;
			if (UKismetMathLibrary::LinePlaneIntersection_OriginNormal(
				WorldLocation,
				WorldLocation + WorldDirection * 10000.f,
				PlaneOrigin,
				FVector::UpVector,
				T,
				Intersection))
			{
				return Intersection;
			}
		}
	}
	return GetActorLocation();
}

