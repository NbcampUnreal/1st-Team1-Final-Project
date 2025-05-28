// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_MerciAimingSkill.generated.h"

class AGS_SeekerMerciArrow;
/**
 * 
 */
UCLASS()
class GAS_API UGS_MerciAimingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	UGS_MerciAimingSkill();
	virtual void ActiveSkill() override;
	virtual void OnSkillCommand() override;
	virtual bool CanActive() const override;

	virtual bool CanActiveInternally() const;

	TSubclassOf<AGS_SeekerMerciArrow> ArrowClass;
private:
	bool bPressedDuringCooldown = false;
};
