// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_SeekerSkillBase.generated.h"

class AGS_Monster;
class AGS_Guardian;
/**
 * 
 */
UCLASS()
class GAS_API UGS_SeekerSkillBase : public UGS_SkillBase
{
	GENERATED_BODY()
	
protected:
	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target);
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target);
	virtual void ExecuteSkillEffect() override;
};
