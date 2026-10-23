// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDSWeaponTypes.h"
#include "TDSWeapon.generated.h"

class USkeletalMeshComponent;
class ACharacter;
class USphereComponent;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName MuzzleSocketName = TEXT("Muzzle_Socket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName CharacterGripSocketName = TEXT("Pistol_Grip_socket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName LeftHandSocketName = TEXT("LeftHandSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	bool bUseLeftHandIK = true;

	// ====Combat State (BP_Pistol / BP_AR) ====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float FireRate = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float ProjectileSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float MinAimDistance = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	float WallClearance = 40.f;

	// ==== Functions ====
	/** Attach Weapon into Character's hand */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(ACharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Holster();

	//** Drop the weapon */ 
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DropWeapon(const FVector& Location, const FRotator& Rotation);

	/** Fire (if attached) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetPickupEnabled(bool bEnabled);

	//** bIsEquipped */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsEquipped() const { return bIsEquipped; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool UsesLeftHandIK() const { return bUseLeftHandIK && bIsEquipped; }

	// Weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Pistol;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetWeaponType() const { return WeaponType; }

protected:
	UFUNCTION()
	void OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPickupEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool CanFire() const;

	bool bIsEquipped = false;
	float NextFireTime = 0.f;

};
