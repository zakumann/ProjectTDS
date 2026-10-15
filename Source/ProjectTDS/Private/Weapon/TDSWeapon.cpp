// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/TDSWeapon.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ATDSWeapon::ATDSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	SetRootComponent(WeaponMesh);

	MuzzleSocketName = TEXT("Muzzle_Socket");

	CharacterGripSocketName = TEXT("Pistol_Grip_socket");


}

// Called when the game starts or when spawned
void ATDSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

/*// Called every frame
void ATDSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

void ATDSWeapon::EquipWeapon(ACharacter* TargetCharacter)
{
	if (TargetCharacter == nullptr) return;

	// Reset Weapon and Collision deactivate 
	SetOwner(TargetCharacter);

	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Bring on Skeletal Mesh of Character
	USkeletalMeshComponent* CharacterMesh = TargetCharacter->GetMesh();
	if (CharacterMesh)
	{
		// Snap to Target(Matching with Weapon for Location of Socket and Rotation)
		FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);

		// Weapon Actor attaching where the name  'Grip_Socket' in Character's hand 
		AttachToComponent(CharacterMesh, AttachRules, CharacterGripSocketName);
	}
}

void ATDSWeapon::Fire()
{

}

