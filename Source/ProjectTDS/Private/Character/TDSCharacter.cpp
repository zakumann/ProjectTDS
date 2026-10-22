// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TDSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/TDSWeapon.h"

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

	Tags.AddUnique(FName("Player")); // Add "Player" Tag for not overdrived by names.
}

// Called when the game starts or when spawned
void ATDSCharacter::BeginPlay()
{
	Super::BeginPlay();

	for (TSubclassOf<ATDSWeapon> WeaponClass : StartingWeaponClasses)
	{
		if (!WeaponClass) continue;
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		if (ATDSWeapon* NewWeapon = GetWorld()->SpawnActor<ATDSWeapon>(WeaponClass, GetActorTransform(), SpawnParams))
		{
			NewWeapon->Holster();
			WeaponInventory.Add(NewWeapon);
		}
	}

	// Attach Starting Weapon
	if (WeaponInventory.Num() > 0)
	{
		SwitchWeapon(0);
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

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDSCharacter::Move);
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ATDSCharacter::Interact);

		// Interact
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ATDSCharacter::Interact);
		EnhancedInput->BindAction(SwitchWeapon1Action, ETriggerEvent::Started, this, &ATDSCharacter::SwitchToWeapon1);
		EnhancedInput->BindAction(SwitchWeapon2Action, ETriggerEvent::Started, this, &ATDSCharacter::SwitchToWeapon2);
		EnhancedInput->BindAction(DropWeaponAction, ETriggerEvent::Started, this, &ATDSCharacter::DropCurrentWeapon);
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

void ATDSCharacter::Shoot(const FInputActionValue& Value)
{
	if (CurrentWeapon && CurrentWeapon->IsEquipped())
	{
		CurrentWeapon->Fire();
	}
}

void ATDSCharacter::DropCurrentWeapon(const FInputActionValue& Value)
{
	if (!CurrentWeapon || !WeaponInventory.IsValidIndex(CurrentWeaponIndex)) return;

	ATDSWeapon* WeaponToDrop = CurrentWeapon;
	const int32 RemovedIndex = CurrentWeaponIndex;

	CurrentWeapon = nullptr;
	CurrentWeaponIndex = INDEX_NONE;
	WeaponInventory.RemoveAt(RemovedIndex);

	const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 120.f;
	const FRotator DropRotation(0.f, GetActorRotation().Yaw, 0.f);
	WeaponToDrop->DropWeapon(DropLocation, DropRotation);

	if (WeaponInventory.Num() > 0)
	{
		SwitchWeapon(0);
	}
}

void ATDSCharacter::Interact(const FInputActionValue& Value)
{
	if (!OverlappingWeapon || OverlappingWeapon->IsEquipped()) return;

	ATDSWeapon* Picked = OverlappingWeapon;
	OverlappingWeapon = nullptr;
	AddWeaponToInventory(Picked);
}

void ATDSCharacter::SwitchToWeapon1(const FInputActionValue& Value)
{
	SwitchWeapon(0);
}

void ATDSCharacter::SwitchToWeapon2(const FInputActionValue & Value)
{
	SwitchWeapon(1);
}



void ATDSCharacter::AddWeaponToInventory(ATDSWeapon* NewWeapon)
{
	if (!NewWeapon || NewWeapon->IsEquipped()) return;
	if (WeaponInventory.Contains(NewWeapon)) return;

	if (WeaponInventory.Num() >= MaxWeaponSlots)
	{
		if (!CurrentWeapon)
		{
			return;
		}

		ATDSWeapon* WeaponToDrop = CurrentWeapon;
		const int32 RemovedIndex = CurrentWeaponIndex;

		CurrentWeapon = nullptr;
		CurrentWeaponIndex = INDEX_NONE;
		WeaponInventory.RemoveAt(RemovedIndex);

		const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 120.f;
		const FRotator DropRotation(0.f, GetActorRotation().Yaw, 0.f);
		WeaponToDrop->DropWeapon(DropLocation, DropRotation);
	}

	NewWeapon->Holster();
	WeaponInventory.Add(NewWeapon);

	// if This weapon equip first, attach this.
	SwitchWeapon(WeaponInventory.Num() - 1);
}

void ATDSCharacter::SwitchWeapon(int32 Index)
{
	if (!WeaponInventory.IsValidIndex(Index)) return;
	if (Index == CurrentWeaponIndex && CurrentWeapon && CurrentWeapon->IsEquipped()) return;

	// CurrentWeapon Hide / disarmed
	UnequipCurrentWeapon();

	CurrentWeaponIndex = Index;
	CurrentWeapon = WeaponInventory[Index];

	if (CurrentWeapon)
	{
		CurrentWeapon->EquipWeapon(this);
	}
}

void ATDSCharacter::UnequipCurrentWeapon()
{
	if (!CurrentWeapon) return;

	// Detaching weapon
	CurrentWeapon->Holster();
	CurrentWeapon = nullptr;
	CurrentWeaponIndex = INDEX_NONE;
}



void ATDSCharacter::SetOverlappingWeapon(ATDSWeapon* Weapon)
{
	if (Weapon && !Weapon->IsEquipped())
	{
		OverlappingWeapon = Weapon;
	}
}

void ATDSCharacter::ClearOverlappingWeapon(ATDSWeapon* Weapon)
{
	if (OverlappingWeapon == Weapon)
	{
		OverlappingWeapon = nullptr;
	}
}

EWeaponType ATDSCharacter::GetCurrentWeaponType() const
{
	return (CurrentWeapon && CurrentWeapon->IsEquipped())
		? CurrentWeapon->GetWeaponType() : EWeaponType::Unarmed;
}
