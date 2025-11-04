// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_SeekerSkillBase.h"
#include "GS_SeekerRollSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_SeekerRollSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
public:
	UGS_SeekerRollSkill();
	virtual void ActiveSkill() override;
	virtual void OnSkillCanceledByDebuff() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void InterruptSkill() override;

	FOnMontageEnded EndDelegate;

	UFUNCTION()
	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};