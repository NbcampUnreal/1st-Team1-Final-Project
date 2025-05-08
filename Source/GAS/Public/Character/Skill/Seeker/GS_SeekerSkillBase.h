// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_SeekerSkillBase.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_SeekerSkillBase : public UGS_SkillBase
{
	GENERATED_BODY()
	
protected:
	virtual void ActiveSkill() override;
	virtual void ApplyEffectToBoss(AActor* Target);
	virtual void ApplyEffectToDungeonMonster(AActor* Target);
	virtual void ExecuteSkillEffect() override;
};
