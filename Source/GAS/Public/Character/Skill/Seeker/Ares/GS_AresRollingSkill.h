// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_AresRollingSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_AresRollingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
public:
	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
};
