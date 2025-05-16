#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GS_DrakharAnimInstance.generated.h"

class AGS_Drakhar;

DECLARE_MULTICAST_DELEGATE(FOnNextAttackCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAttackHitCheckDelegate);

UCLASS()
class GAS_API UGS_DrakharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UGS_DrakharAnimInstance();

	virtual void NativeInitializeAnimation() override;

	//combo attack
	FOnNextAttackCheckDelegate OnNextAttackCheck;
	FOnAttackHitCheckDelegate  OnAttackHitCheck;

	//[combo attack]
	void PlayComboAttackMontage(int32 InCurrentComboIndex);
	void StopComboAttackMontage(int32 InCurrentComboIndex);

	//dash skill
	void PlayDashMontage();
	void StopDashMontage();

	//earthquake
	void PlayEarthquakeMontage();
	void StopEarthquakeMontage();

private:
	UPROPERTY()
	TObjectPtr<AGS_Drakhar> Drakhar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TArray<UAnimMontage*> ComboAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DashMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* EarthquakeMontage;

	//[combo attack]
	UFUNCTION()
	void AnimNotify_ComboAttackCheck();
	
	UFUNCTION()
	void AnimNotify_ComboAttackEnd();

	//[dash skill]
	UFUNCTION()
	void AnimNotify_DashHitCheck();

	//[earthquake skill]
	UFUNCTION()
	void AnimNotify_EarthquakeCheck();

	UFUNCTION()
	void AnimNotify_EarthquakeCheckEnd();
};
