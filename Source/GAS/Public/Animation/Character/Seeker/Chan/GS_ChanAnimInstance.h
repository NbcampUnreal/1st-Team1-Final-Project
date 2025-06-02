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

	// Chan Combo Attack
	/*UFUNCTION()
	void AnimNotify_ComboInputOpen();
	UFUNCTION()
	void AnimNotify_CheckToNext();
	UFUNCTION()
	void AnimNotify_ComboEnd();*/
	
	/*UPROPERTY(BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingUpperBodyMontage = false;

	UPROPERTY(BlueprintReadWrite, Category = "Montage", meta = (BlueprintThreadSafe))
	bool IsPlayingFullBodyMontage = false;*/
};
