// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TDSEnemyBase.h"

// Sets default values
ATDSEnemyBase::ATDSEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default state
	MaxHealth = 100.f;
	Health = MaxHealth;

	Tags.AddUnique(FName("Enemy"));
}

// Called when the game starts or when spawned
void ATDSEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

float ATDSEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage <= 0.f || Health <= 0.f) return 0.f;

	// Decrease health
	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);

	// If Health equal with 0 then die
	if (Health <= 0.f)
	{
		HandleDeath();
	}

	return ActualDamage;
}
void ATDSEnemyBase::HandleDeath()
{
	// TODO: Here can add Death animation, Kill scores, and Item drop event.

	// After all, Died enemy character clear on the field.
	Destroy();
}