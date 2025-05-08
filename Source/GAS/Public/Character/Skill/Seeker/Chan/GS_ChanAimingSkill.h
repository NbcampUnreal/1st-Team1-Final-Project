// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_ChanAimingSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanAimingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	virtual void ActiveSkill() override;
	virtual void ExecuteSkillEffect() override;

	// 공격
	void OnShieldSlam();

	// 스탠스 관리
	void TickDrainStamina();

protected:
	void StartHoldUp();
	void EndHoldUp();

	virtual void ApplyEffectToDungeonMonster() override;
	virtual void ApplyEffectToBoss() override;

private:

	// 스테미나 관리
	bool bIsHoldingUp;
	float MaxStamina;
	float CurrentStamina;
	float StaminaDrainRate; // 초당 소비량
	float SlamStaminaCost;
	FTimerHandle StaminaDrainHandle;
};
