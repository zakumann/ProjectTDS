// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/TDSAnimInstance.h"
#include "Character/TDSCharacter.h"
#include "Weapon/TDSWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTDSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const ATDSCharacter* Character = Cast<ATDSCharacter>(TryGetPawnOwner());
	if (!Character) return;

	const FVector Velocity = Character->GetVelocity();
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	bIsMoving = GroundSpeed > 3.f;

	WeaponType = Character->GetCurrentWeaponType();
	bUseLeftHandIK = false;

	if (const ATDSWeapon* Weapon = Character->GetCurrentWeapon())
	{
		if (Weapon->IsEquipped() && Weapon->UsesLeftHandIK() && Weapon->WeaponMesh)
		{
			bUseLeftHandIK = true;
			LeftHandEffectorLocation = Weapon->WeaponMesh->GetSocketLocation(Weapon->LeftHandSocketName);
		}
	}
}