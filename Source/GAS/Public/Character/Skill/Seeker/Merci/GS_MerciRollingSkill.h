// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_MerciRollingSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_MerciRollingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
public:
	UGS_MerciRollingSkill();
	virtual void ActiveSkill() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void InterruptSkill() override;
};
