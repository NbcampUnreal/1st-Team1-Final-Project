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

	//combo attack
	void PlayAttackMontage();
	void JumpToAttackMontageSection(int32 NewSection);

	//dash skill
	void PlayDashMontage();
	void StopDashMontage();

	//earthquake
	void PlayEarthquakeMontage();
	void StopEarthquakeMontage();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DashMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* EarthquakeMontage;

	//dash skill
	//float DashMontageDuration;

	UFUNCTION()
	void AnimNotify_AttackHitCheck();

	UFUNCTION()
	void AnimNotify_NextAttackCheck();

	UFUNCTION()
	void AnimNotify_DashHitCheck();

	UFUNCTION()
	void AnimNotify_EarthquakeCheck();

	UFUNCTION()
	void AnimNotify_EarthquakeCheckEnd();

	FName GetAttackMontageSectionName(int32 Section);
};
