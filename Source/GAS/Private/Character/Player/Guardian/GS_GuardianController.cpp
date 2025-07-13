#include "Character/Player/Guardian/GS_GuardianController.h"

#include "Character/Player/Guardian/GS_Guardian.h"

#include "EnhancedInputComponent.h"

AGS_GuardianController::AGS_GuardianController()
{
	
}

void AGS_GuardianController::CtrlInput(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());

		if (IsValid(Guardian))
		{
			Guardian->Ctrl();
		}
	}
}

void AGS_GuardianController::CtrlStop(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());

		if (IsValid(Guardian))
		{
			Guardian->CtrlStop();
		}
	}
}

void AGS_GuardianController::LeftMouseInput(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());
		if (IsValid(Guardian))
		{
			Guardian->LeftMouse();
		}
	}
}

void AGS_GuardianController::RightMouseInput(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());

		if (IsValid(Guardian))
		{
			Guardian->RightMouse();
		}
	}
}

void AGS_GuardianController::BeginPlay()
{
	Super::BeginPlay();
}

void AGS_GuardianController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	if (IsValid(EnhancedInputComponent))
	{
		if (IsValid(CtrlInputAction))
		{
			EnhancedInputComponent->BindAction(CtrlInputAction, ETriggerEvent::Triggered, this, &AGS_GuardianController::CtrlInput);
			EnhancedInputComponent->BindAction(CtrlInputAction, ETriggerEvent::Completed, this, &AGS_GuardianController::CtrlStop);
		}
		if (IsValid(LeftMouseInputAction))
		{
			EnhancedInputComponent->BindAction(LeftMouseInputAction, ETriggerEvent::Started, this, &AGS_GuardianController::LeftMouseInput);
		}
		if (IsValid(RightMouseInputAction))
		{
			EnhancedInputComponent->BindAction(RightMouseInputAction, ETriggerEvent::Started, this, &AGS_GuardianController::RightMouseInput);
		}
	}
}
