// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_AresAimingSkill.generated.h"

class AGS_SwordAuraProjectile;

UCLASS()
class GAS_API UGS_AresAimingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_AresAimingSkill();

	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;	
	virtual void ExecuteSkillEffect() override;
	virtual bool IsActive() const override;
	
private:
	UPROPERTY()
	AGS_SwordAuraProjectile* CachedProjectile;

	FTimerHandle DelaySecondProjectileHandle;
	void SpawnSecondProjectile();

};
