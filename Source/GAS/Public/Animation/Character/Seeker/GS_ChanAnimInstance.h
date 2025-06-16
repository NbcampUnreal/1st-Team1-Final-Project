// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "GS_ChanAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanAnimInstance : public UGS_SeekerAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// Combo Attack
	UFUNCTION()
	void AnimNotify_ComboInput();
	UFUNCTION()
	void AnimNotify_CanProceed();
	UFUNCTION()
	void AnimNotify_ComboEnd();
};
