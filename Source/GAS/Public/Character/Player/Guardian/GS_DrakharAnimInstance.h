#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GS_DrakharAnimInstance.generated.h"

class AGS_Drakhar;

UCLASS()
class GAS_API UGS_DrakharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UGS_DrakharAnimInstance();

	virtual void NativeInitializeAnimation() override;
	
private:
	UPROPERTY()
	TObjectPtr<AGS_Drakhar> Drakhar;

	UFUNCTION()
	void AnimNotify_ComboAttack();

	UFUNCTION()
	void AnimNotify_Reset();

	UFUNCTION()
	void AnimNotify_ShootEnergy();

	//[earthquake skill]
	UFUNCTION()
	void AnimNotify_EarthquakeCheck();

	//[draconic fury]
	UFUNCTION()
	void AnimNotify_DraconicFury();

	//[skill check]
	UFUNCTION()
	void AnimNotify_CtrlSkillEnd();
};
