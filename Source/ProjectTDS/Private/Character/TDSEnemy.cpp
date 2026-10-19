// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/TDSEnemy.h"
#include "Weapon/TDSWeapon.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATDSEnemy::ATDSEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Tags.AddUnique(FName("Enemy"));
}

// Called when the game starts or when spawned
void ATDSEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (StartingWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		CurrentWeapon = GetWorld()->SpawnActor<ATDSWeapon>(StartingWeaponClass, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->EquipWeapon(this);
		}
	}
}

// Called every frame
void ATDSEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryFireAtPlayer();
}

ACharacter* ATDSEnemy::FindPlayer() const
{
	return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

void ATDSEnemy::TryFireAtPlayer()
{
	if (!CurrentWeapon || !CurrentWeapon->IsEquipped()) return;
	if (GetWorld()->GetTimeSeconds() < NextFireTime) return;

	ACharacter* Player = FindPlayer();
	if (!Player) return;

	const float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (Distance > AttackRange) return;

	// Rotate to the Player
	FVector Direction = Player->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;
	FRotator LookAtRotation = Direction.Rotation();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 5.f));

	CurrentWeapon->Fire();
	NextFireTime = GetWorld()->GetTimeSeconds() + FireInterval;
}
