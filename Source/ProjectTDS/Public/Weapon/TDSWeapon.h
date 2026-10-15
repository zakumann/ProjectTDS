// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDSWeapon.generated.h"

class USkeletalMeshComponent;

UCLASS()
class PROJECTTDS_API ATDSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATDSWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
/*	// Called every frame
	virtual void Tick(float DeltaTime) override;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName CharacterGripSocketName;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(ACharacter* TargetCharacter);

	virtual void Fire();
};
