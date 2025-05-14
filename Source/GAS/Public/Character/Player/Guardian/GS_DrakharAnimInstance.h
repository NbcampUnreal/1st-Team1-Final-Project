#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GS_DrakharAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnNextAttackCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAttackHitCheckDelegate);

UCLASS()
class GAS_API UGS_DrakharAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UGS_DrakharAnimInstance();

	//combo attack
	FOnNextAttackCheckDelegate OnNextAttackCheck;
	FOnAttackHitCheckDelegate  OnAttackHitCheck;

	//[combo attack]
	//legacy
	void PlayAttackMontage();
	void JumpToAttackMontageSection(int32 NewSection);
	//new
	void PlayComboAttackMontage(int32 InCurrentComboIndex);
	void StopComboAttackMontage(int32 InCurrentComboIndex);

	//dash skill
	void PlayDashMontage();
	void StopDashMontage();

	//earthquake
	void PlayEarthquakeMontage();
	void StopEarthquakeMontage();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TArray<UAnimMontage*> ComboAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DashMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* EarthquakeMontage;

	//[combo attack]
	UFUNCTION()
	void AnimNotify_ComboAttackCheckStart();
	UFUNCTION()
	void AnimNotify_ComboAttackCheckEnd();

	//[dash skill]
	UFUNCTION()
	void AnimNotify_DashHitCheck();

	//[earthquake skill]
	UFUNCTION()
	void AnimNotify_EarthquakeCheck();

	UFUNCTION()
	void AnimNotify_EarthquakeCheckEnd();

};
