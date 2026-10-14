// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/TDSPlayerController.h"
#include "EnhancedInputSubsystems.h"

void ATDSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// If isLocalPlayerController
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (const TObjectPtr<UInputMappingContext>& Context : InputMappingContexts)
			{
				if (Context)
				{
					Subsystem->AddMappingContext(Context, 0);
				}
			}
		}
	}

	bShowMouseCursor = true; // Show Mouse cursor on screen
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}
