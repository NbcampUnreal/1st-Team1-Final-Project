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
	virtual void OnSkillCommand() override;
	virtual void ExecuteSkillEffect() override;
	virtual bool IsActive() const override;

	// 공격
	void OnShieldSlam();

	// 스테미나 관리
	void TickDrainStamina();

protected:
	// 스탠스 관리
	void StartHoldUp();
	void EndHoldUp();

	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	// 스탠스 관리
	bool bIsHoldingUp;

	// 스테미나 관리
	float MaxStamina=20.0f;
	float CurrentStamina;
	float StaminaDrainRate=1.0f; // 초당 소비량
	float SlamStaminaCost=5.0f;
	FTimerHandle StaminaDrainHandle;
};
