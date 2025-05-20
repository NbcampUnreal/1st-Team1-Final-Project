// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "GS_SmokeFieldSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_SmokeFieldSkill : public AGS_FieldSkillActor
{
	GENERATED_BODY()
protected:
	virtual void ApplyFieldEffectToMonster(AGS_Monster* Target);
	virtual void RemoveFieldEffectFromMonster(AGS_Monster* Target);
	virtual void ApplyFieldEffectToGuardian(AGS_Guardian* Target);
	virtual void RemoveFieldEffectFromGuardian(AGS_Guardian* Target);
};
