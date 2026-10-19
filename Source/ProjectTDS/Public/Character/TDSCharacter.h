// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TDSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class ATDSWeapon;

UCLASS()
class PROJECTTDS_API ATDSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATDSCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> TopDownCameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DropWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchWeapon1Action;	// 1

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SwitchWeapon2Action; // 2

	// Starting Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<TSubclassOf<ATDSWeapon>> StartingWeaponClasses;

	// Weapon Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<ATDSWeapon>> WeaponInventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ATDSWeapon> CurrentWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentWeaponIndex = INDEX_NONE;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ATDSWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	void SetOverlappingWeapon(ATDSWeapon* Weapon);
	void ClearOverlappingWeapon(ATDSWeapon* Weapon);

protected:
	void Interact(const FInputActionValue& Value);
	void DropCurrentWeapon(const FInputActionValue& Value);
	void SwitchToWeapon1(const FInputActionValue& Value);
	void SwitchToWeapon2(const FInputActionValue& Value);

	void AddWeaponToInventory(ATDSWeapon* NewWeapon);
	void SwitchWeapon(int32 Index);
	void UnequipCurrentWeapon();

	UPROPERTY()
	TObjectPtr<ATDSWeapon> OverlappingWeapon;

protected:
	void Move(const FInputActionValue& Value);
	void RotateToMouseCursor();
	void Shoot(const FInputActionValue& Value);
private:
	FVector GetMouseCursorWorldLocation() const;
};
