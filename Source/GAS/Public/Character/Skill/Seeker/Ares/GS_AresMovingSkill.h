// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_AresMovingSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_AresMovingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
	virtual void OnSkillCommand() override;
	virtual void ExecuteSkillEffect() override;
	virtual bool IsActive() const override;

protected:
	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	void UpdateCharging();

	FTimerHandle ChargingTimerHandle;

	float ChargingTime = 0.0f;
	float ChargingStartTime = 0.0f;
	float MaxChargingTime = 2.0f;

	float MinDashDistance = 600.0f;
	float MaxDashDistance = 2000.0f;

	bool bIsCharging = false;

	FVector DashDirection; // 돌진 방향
	FVector DashStartLocation;
	FVector DashEndLocation;
};
