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

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ProjectileTrace), false, this);
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(GetInstigator());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		LastLocation,
		CurrentLocation,
		ECC_WorldStatic,
		Params);

	LastLocation = CurrentLocation;

	if (bHit)
	{
		SetActorLocation(Hit.ImpactPoint);
		Destroy();
	}
}

void ATDSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator()) // ! If not valid OtherActor or Otheractor is mine or if another is owner is mine or OtherActor is Instigator..
	{
		Destroy();
		return;
	}

	AActor* Shooter = GetInstigator() ? static_cast<AActor*>(GetInstigator()) : GetOwner();

	bool bShouldDamage = false;

	// Player to Enemies
	if (Shooter && Shooter->ActorHasTag(FName("Player")))
	{
		if (OtherActor->ActorHasTag(FName("Enemy"))) // If OtherActor Tag is Enemy
		{
			bShouldDamage = true; // Give damage to Enemy success
		}
	}
	else if (Shooter && Shooter->ActorHasTag(FName("Enemy")))
	{
		if (OtherActor->ActorHasTag(FName("Player")))
		{
			bShouldDamage = true;
		}
	}

	if (bShouldDamage)
	{
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);
	}

	Destroy();
}

