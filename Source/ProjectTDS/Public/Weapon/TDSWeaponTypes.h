// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDSWeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed UMETA(DisplayName = "Unarmed"),
	Pistol	UMETA(DisplayName = "Pistol"),
	AR		UMETA(DisplayName = "AR")
};
/**
 * 
 */
class PROJECTTDS_API TDSWeaponTypes
{

};
