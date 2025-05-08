#include "Character/Player/Guardian/GS_GuardianController.h"

#include "Character/Player/Guardian/GS_Guardian.h"

#include "EnhancedInputComponent.h"

AGS_GuardianController::AGS_GuardianController()
	:TestAttackAction(nullptr)
{

}

void AGS_GuardianController::TestAttack(const FInputActionValue& InputValue)
{
	AGS_Guardian* Guardian = Cast<AGS_Guardian>(GetPawn());
	if (IsValid(Guardian))
	{
		UE_LOG(LogTemp, Warning, TEXT("Test Attack"));

		Guardian->TestMeleeAttack();
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
	if (TestAttackAction)
	{
		EnhancedInputComponent->BindAction(TestAttackAction, ETriggerEvent::Started, this, &AGS_GuardianController::TestAttack);
	}
}
