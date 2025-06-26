// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_DrakharEarthquake.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_DrakharEarthquake : public UGS_SkillBase
{
	GENERATED_BODY()
	
public:
	UGS_DrakharEarthquake();

	virtual void ActiveSkill() override;
	virtual void ExecuteSkillEffect() override;

protected:
	bool bIsEarthquaking;
};
