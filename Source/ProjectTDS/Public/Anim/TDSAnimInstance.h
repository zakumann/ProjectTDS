// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Weapon/TDSWeaponTypes.h"
#include "TDSAnimInstance.generated.h"

class ATDSCharacter;

/**
 * 
 */
UCLASS()
class PROJECTTDS_API UTDSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	// Defalt Armed state
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Unarmed;
	// LeftHandIK for weapon
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bUseLeftHandIK = false;
	// Place left hand skeleton point
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FVector LeftHandEffectorLocation = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving = false;
};
