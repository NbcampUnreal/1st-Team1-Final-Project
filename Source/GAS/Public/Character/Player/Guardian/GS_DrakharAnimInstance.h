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

	//[combo attack]
	void PlayComboAttackMontage(int32 InCurrentComboIndex);
	void StopComboAttackMontage(int32 InCurrentComboIndex);
	
private:
	UPROPERTY()
	TObjectPtr<AGS_Drakhar> Drakhar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TArray<UAnimMontage*> ComboAttackMontages;

	//[combo attack]
	UFUNCTION()
	void AnimNotify_ComboAttackCheck();
	
	UFUNCTION()
	void AnimNotify_ComboAttackEnd();

	//[earthquake skill]
	UFUNCTION()
	void AnimNotify_EarthquakeCheck();

	//[draconic fury]
	UFUNCTION()
	void AnimNotify_DraconicFury();

	UFUNCTION()
	void AnimNotify_SkillStop();
};
