// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "GS_AresAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_AresAnimInstance : public UGS_SeekerAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
