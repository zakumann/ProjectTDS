// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDSEnemy.generated.h"

class ATDSWeapon;

UCLASS()
class PROJECTTDS_API ATDSEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDSEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ATDSWeapon> StartingWeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ATDSWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float FireInterval = 1.5f;

protected:
	float NextFireTime = 0.f;

	void TryFireAtPlayer();
	ACharacter* FindPlayer() const;
};
