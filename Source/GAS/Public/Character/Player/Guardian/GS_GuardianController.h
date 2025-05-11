#pragma once

#include "CoreMinimal.h"
#include "Character/GS_TpsController.h"
#include "GS_GuardianController.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_GuardianController : public AGS_TpsController
{
	GENERATED_BODY()
	
public:
	AGS_GuardianController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ComboAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* Skill1Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* Skill2Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* UltimateSkillAction;

	UFUNCTION()
	void ComboAttack(const FInputActionValue& InputValue);

	UFUNCTION()
	void Skill1(const FInputActionValue& InputValue);

	UFUNCTION()
	void Skill2(const FInputActionValue& InputValue);

	UFUNCTION()
	void UltimateSkill(const FInputActionValue& InputValue);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};
