// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/TDSWeapon.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Character/TDSCharacter.h"

// Sets default values
ATDSWeapon::ATDSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);

	PickupCollision = CreateDefaultSubobject<USphereComponent>("PickupCollision");
	PickupCollision->SetupAttachment(WeaponMesh);
	PickupCollision->InitSphereRadius(80.f); // Pickup Radius
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Only overlap with Player
	PickupCollision->SetGenerateOverlapEvents(true);

	MuzzleSocketName = TEXT("Muzzle_Socket");
	CharacterGripSocketName = TEXT("Pistol_Grip_socket");
}

// Called when the game starts or when spawned
void ATDSWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponMesh)
	{
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	

	SetPickupEnabled(!bIsEquipped);

	if (PickupCollision)
	{
		PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &ATDSWeapon::OnPickupBeginOverlap);
		PickupCollision->OnComponentEndOverlap.AddDynamic(this, &ATDSWeapon::OnPickupEndOverlap);
	}
}

/*// Called every frame
void ATDSWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

void ATDSWeapon::SetPickupEnabled(bool bEnabled)
{
	if (!PickupCollision) return;

	PickupCollision->SetGenerateOverlapEvents(bEnabled);
	PickupCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ATDSWeapon::EquipWeapon(ACharacter* TargetCharacter)
{
	if (TargetCharacter == nullptr) return;

	// Reset Weapon and Collision deactivate 
	SetOwner(TargetCharacter); // Who Owning it.
	SetInstigator(TargetCharacter); // Who did it 
	bIsEquipped = true;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetPickupEnabled(false);

	// Bring on Skeletal Mesh of Character
	USkeletalMeshComponent* CharacterMesh = TargetCharacter->GetMesh();
	if (!CharacterMesh) return;

	FName SocketToUse = CharacterGripSocketName;

	if (!CharacterMesh->DoesSocketExist(SocketToUse))
	{
		// If there is not socket fallback
		const FName HandSocket(TEXT("hand_r"));
		SocketToUse = CharacterMesh->DoesSocketExist(HandSocket) ? HandSocket : FName(NAME_None);
	}
		// Weapon Actor attaching where the name  'Grip_Socket' in Character's hand 
		AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketToUse);
}

void ATDSWeapon::Holster()
{
	bIsEquipped = false;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetPickupEnabled(false);
}

void ATDSWeapon::DropWeapon(const FVector& Location, const FRotator& Rotation)
{
	// Deactivate Detach
	bIsEquipped = false;

	// Separate from Character
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	SetInstigator(nullptr);

	SetActorLocation(Location);
	SetActorRotation(Rotation);

	// Shows + Collision activate
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	WeaponMesh->SetSimulatePhysics(false);			// If you want turn to true
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Owner / Instigator to Reset
	SetPickupEnabled(true);
	UpdateOverlaps(true);
}

bool ATDSWeapon::CanFire() const
{
	return bIsEquipped &&
		ProjectileClass &&
		GetWorld()->GetTimeSeconds() >= NextFireTime;
}

void ATDSWeapon::Fire()
{
	if (!CanFire()) return; // bool -> If this is can't fire state then over.

	UWorld* World = GetWorld();
	FVector SpawnLocation = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketLocation(MuzzleSocketName)
		: WeaponMesh->GetComponentLocation() + WeaponMesh->GetForwardVector() * 50.f;
	FRotator SpawnRotation = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketRotation(MuzzleSocketName)
		: WeaponMesh->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (World->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		NextFireTime = World->GetTimeSeconds() + (1.f / FMath::Max(FireRate, 0.01f));
	}
}


void ATDSWeapon::OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsEquipped) return; // If already weapon equipped ignore it.

	if (ATDSCharacter* Player = Cast<ATDSCharacter>(OtherActor))
	{
		Player->SetOverlappingWeapon(this); // To Character "You can pickup this weapon."
	}
}

void ATDSWeapon::OnPickupEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ATDSCharacter* Player = Cast<ATDSCharacter>(OtherActor))
	{
		Player->ClearOverlappingWeapon(this);
	}
}