// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/TDSProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ATDSProjectile::ATDSProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>("CollisionComp");
	CollisionComp->InitSphereRadius(6.f);
	// When Projectile Hit something-> On Hit!
	CollisionComp->OnComponentHit.AddDynamic(this, &ATDSProjectile::OnHit);
	// Set Collision and physics
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->SetGenerateOverlapEvents(true);

	SetRootComponent(CollisionComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(0.25f));


	// I Saw this code. However, I think this is not important : 1) I can't understand why AI generated this. 2) I don't have plan Objectile shape as sphere.
	/** static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMeshAsset.Object);
	}*/

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	InitialLifeSpan = LifeSpanTime;
}

// Called when the game starts or when spawned
void ATDSProjectile::BeginPlay()
{
	Super::BeginPlay();
	LastLocation = GetActorLocation();
	
}

void ATDSProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector CurrentLocation = GetActorLocation();
	if (LastLocation.Equals(CurrentLocation, 0.1f))
	{
		return;
	}

	// Blocking Clipping situation when Projectile fire due to too high speed.
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ProjectileTrace), false, this);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(GetInstigator());

	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_WorldStatic);
	Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
	Objects.AddObjectTypesToQuery(ECC_Pawn);

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		Hit,
		LastLocation,
		CurrentLocation,
		Objects,
		Params);

	if (bHit)
	{
		SetActorLocation(Hit.ImpactPoint);
		OnHit(CollisionComp, Hit.GetActor(), Hit.GetComponent(), FVector::ZeroVector, Hit);
		return;
	}

	LastLocation = CurrentLocation;
}

void ATDSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) // ! If not valid OtherActor or Otheractor is mine or if another is owner is mine or OtherActor is Instigator..
	{
		Destroy();
		return;
	}

	AActor* Shooter = GetInstigator() ? static_cast<AActor*>(GetInstigator()) : GetOwner();
	const bool bPlayerShot = Shooter && Shooter->ActorHasTag(FName("Player"));
	const bool bEnemyShot = Shooter && Shooter->ActorHasTag(FName("Enemy"));

	if ((bPlayerShot && OtherActor->ActorHasTag(FName("Enemy"))) || (bEnemyShot && OtherActor->ActorHasTag(FName("Player"))))
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
	}

	Destroy();
}

