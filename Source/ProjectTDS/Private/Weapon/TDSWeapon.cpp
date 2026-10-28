// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/TDSWeapon.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Character/TDSCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

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
	LeftHandSocketName = TEXT("LeftHandSocket");
	bUseLeftHandIK = true;
}

// Called when the game starts or when spawned
void ATDSWeapon::BeginPlay()
{
	Super::BeginPlay();
	// Check Collision state when start this.
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
	// Check state and deactivate pick up
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
	// Separte from Character and hidden it.
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
	// 1. Equip state 2. Projectile Class 3. Next fire cooltime.
	return bIsEquipped &&
		ProjectileClass &&
		GetWorld()->GetTimeSeconds() >= NextFireTime;
}

void ATDSWeapon::Fire()
{
	if (!CanFire()) return; // bool -> If this is can't fire state then over.

	UWorld* World = GetWorld();
	if (!World || !ProjectileClass) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	// Fire location
	FVector MuzzleLocation = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketLocation(MuzzleSocketName)
		: GetActorLocation() + GetActorForwardVector() * 50.f;

	// Mouse cursor(Definite : Character)
	FVector AimLocation = MuzzleLocation + GetActorForwardVector() * 1000.f; // Fallback

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		FVector WorldLocation, WorldDirection;
		if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			// Character height and Plane
			const FVector PlaneOrigin(OwnerPawn->GetActorLocation().X, OwnerPawn->GetActorLocation().Y, OwnerPawn->GetActorLocation().Z);
			float T;
			FVector Intersection;
			if (UKismetMathLibrary::LinePlaneIntersection_OriginNormal(
				WorldLocation,
				WorldLocation + WorldDirection * 10000.f,
				PlaneOrigin,
				FVector::UpVector,
				T,
				Intersection))
			{
				AimLocation = Intersection;
				AimLocation.Z = MuzzleLocation.Z; // Plane aim
			}
		}
	}

	if (FVector::Dist2D(OwnerPawn->GetActorLocation(), AimLocation) < MinAimDistance)
	{
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FireAimTrace), false, this);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(GetInstigator());

	bool bBlockedByWall = false;
	FVector TraceStart = MuzzleLocation;
	const FVector TraceEnd = AimLocation;

	for (int32 i = 0; i < 8; ++i)
	{
		FHitResult Hit;
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

		if (!bHit)
		{
			break;
		}

		AActor* HitActor = Hit.GetActor();
		const bool bIsCharacter = HitActor &&
			(HitActor->ActorHasTag(TEXT("Enemy")) || HitActor->ActorHasTag(TEXT("Player")));

		if (bIsCharacter)
		{
			Params.AddIgnoredActor(HitActor);
			TraceStart = Hit.ImpactPoint + (TraceEnd - TraceStart).GetSafeNormal() * 2.f;
			continue;
		}

		bBlockedByWall = true;
		break;
	}

	if (bBlockedByWall)
	{
		return;
	}
	// FirePoint -> Direction of Cursor
	FVector AimDir = (AimLocation - MuzzleLocation).GetSafeNormal();
	if (AimDir.IsNearlyZero())
	{
		AimDir = GetActorForwardVector();
	}

	const FRotator SpawnRotation = AimDir.Rotation();

	// Spawn Projectile.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Projectile = World->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, SpawnRotation, SpawnParams);
	if (Projectile)
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