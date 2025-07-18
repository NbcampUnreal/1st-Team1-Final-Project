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
	UGS_AresMovingSkill();

	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
	virtual void OnSkillCommand() override;
	virtual void ExecuteSkillEffect() override;
	virtual bool IsActive() const override;
	virtual bool CanActiveInternally() const;
	virtual void InterruptSkill() override;

protected:
	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	void UpdateCharging();
	void StartDash();
	void UpdateDash();
	void StopDash();

	FTimerHandle ChargingTimerHandle;
	FTimerHandle DashTimerHandle;

	float ChargingTime = 0.0f;
	float ChargingStartTime = 0.0f;
	float MaxChargingTime = 2.0f;

	float MinDashDistance = 200.0f;
	float MaxDashDistance = 2000.0f;
	float DashDuration = 0.3f;
	float DashInterpAlpha = 0.0f;

	bool bIsCharging = false;

	FVector DashDirection; // 돌진 방향
	FVector DashStartLocation;
	FVector DashEndLocation;

	TSet<AActor*> DamagedActors;

	bool bPressedDuringCooldown = false;
};
