// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_BasePlayerController.h"
#include "EnhancedInputComponent.h"
#include "UI/Screen/Option/GS_InGameMenuUI.h"

void AGS_BasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	MenuAction = nullptr;
}

void AGS_BasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	if (MenuAction)
	{
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Triggered, this, &AGS_BasePlayerController::OpenMenuUI);
	}
}

void AGS_BasePlayerController::OpenMenuUI(const FInputActionValue& InputValue)
{
	if (!InGameMenuUI)
	{
		if (InGameMenuUIClass)
		{
			InGameMenuUI = CreateWidget<UGS_InGameMenuUI>(this, InGameMenuUIClass);
			InGameMenuUI->AddToViewport(1);
		}
	}
	else
	{
		InGameMenuUI->SetVisibility(ESlateVisibility::Visible);
	}

	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}
