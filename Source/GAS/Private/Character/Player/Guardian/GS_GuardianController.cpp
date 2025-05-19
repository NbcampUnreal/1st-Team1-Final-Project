#include "Character/Player/Guardian/GS_GuardianController.h"

#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"

#include "EnhancedInputComponent.h"

AGS_GuardianController::AGS_GuardianController()
{
	
}

void AGS_GuardianController::ComboAttack(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());

		if (IsValid(Guardian))
		{
			Guardian->ComboAttack();
		}
	}
}

void AGS_GuardianController::Skill1(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());

		if (IsValid(Guardian))
		{
			Guardian->Skill1();
		}
	}
}

void AGS_GuardianController::Skill2(const FInputActionValue& InputValue)
{
	if (IsLocalController())
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());

		if (IsValid(Guardian))
		{
			Guardian->Skill2();
		}
	}
}

void AGS_GuardianController::UltimateSkill(const FInputActionValue& InputValue)
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
		if (IsValid(ComboAttackAction))
		{
			EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &AGS_GuardianController::ComboAttack);
		}
		if (IsValid(Skill1Action))
		{
			EnhancedInputComponent->BindAction(Skill1Action, ETriggerEvent::Started, this, &AGS_GuardianController::Skill1);
		}
		if (IsValid(Skill2Action))
		{
			EnhancedInputComponent->BindAction(Skill2Action, ETriggerEvent::Started, this, &AGS_GuardianController::Skill2);
		}
		if (IsValid(CtrlInputAction))
		{
			EnhancedInputComponent->BindAction(CtrlInputAction, ETriggerEvent::Started, this, &AGS_GuardianController::CtrlInput);
		}
		if (IsValid(RightMouseInputAction))
		{
			EnhancedInputComponent->BindAction(RightMouseInputAction, ETriggerEvent::Started, this, &AGS_GuardianController::RightMouseInput);
		}

	}
}
