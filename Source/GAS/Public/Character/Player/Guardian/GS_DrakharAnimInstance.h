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
	//FORCEINLINE float GetDashMontageDuration() { return DashMontageDuration; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DashMontage;

	//dash skill
	//float DashMontageDuration;

	UFUNCTION()
	void AnimNotify_AttackHitCheck();

	UFUNCTION()
	void AnimNotify_NextAttackCheck();

	UFUNCTION()
	void AnimNotify_DashHitCheck();

	FName GetAttackMontageSectionName(int32 Section);
};
