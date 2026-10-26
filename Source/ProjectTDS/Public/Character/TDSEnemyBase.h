// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDSEnemyBase.generated.h"

UCLASS()
class PROJECTTDS_API ATDSEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDSEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Enemy Character health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float MaxHealth;

	// Enemy character current health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Stats")
	float Health;

	// Unreal Default damage system override function
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Die when health become 0.
	virtual void HandleDeath();
};
