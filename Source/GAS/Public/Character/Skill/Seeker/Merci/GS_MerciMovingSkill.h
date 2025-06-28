// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_MerciMovingSkill.generated.h"

class AGS_SeekerMerciArrow;
/**
 * 
 */
UCLASS()
class GAS_API UGS_MerciMovingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
public:
	UGS_MerciMovingSkill();
	virtual void ActiveSkill() override;
	virtual void OnSkillCommand() override;
	virtual void InterruptSkill() override;
	virtual bool CanActive() const override;

	virtual bool CanActiveInternally() const;

	TSubclassOf<AGS_SeekerMerciArrow> ArrowClass;

private:
	bool bPressedDuringCooldown = false;
};
