// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_ChanReadySkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanReadySkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_ChanReadySkill();

	virtual void ActiveSkill() override;
	virtual void OnSkillCanceledByDebuff() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void InterruptSkill() override;

protected:

private:
	UAnimMontage* ActiveMontage;
	UAnimMontage* TargetMontage;
	virtual void DeactiveSkill() override;

	UFUNCTION()
	void HandleStaminaDepleted(bool bByDamage);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	int32 DeactiveMontageIndex;
};
