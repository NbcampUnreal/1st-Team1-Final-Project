#include "Character/Player/Guardian/GS_GuardianController.h"

#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"

#include "EnhancedInputComponent.h"

AGS_GuardianController::AGS_GuardianController()
{

}

void AGS_GuardianController::ComboAttack(const FInputActionValue& InputValue)
{
	AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());
	if (IsValid(Guardian))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Test Attack"));

		Guardian->MulticastRPCComboAttack();
	}
}

void AGS_GuardianController::Skill1(const FInputActionValue& InputValue)
{

}

void AGS_GuardianController::Skill2(const FInputActionValue& InputValue)
{
}

void AGS_GuardianController::UltimateSkill(const FInputActionValue& InputValue)
{
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
		if (ComboAttackAction)
		{
			EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &AGS_GuardianController::ComboAttack);
		}
	}
}
