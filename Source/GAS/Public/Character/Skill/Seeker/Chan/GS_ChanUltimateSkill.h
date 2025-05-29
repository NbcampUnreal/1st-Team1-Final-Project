// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_ChanUltimateSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanUltimateSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	UGS_ChanUltimateSkill();
	virtual void ActiveSkill() override;
	virtual void ExecuteSkillEffect() override;
};
