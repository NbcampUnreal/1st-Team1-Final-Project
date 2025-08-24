// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_ChanAimingSkill.generated.h"

// Forward declarations
class UGS_SeekerAudioComponent;

/**
 *
 */
UCLASS()
class GAS_API UGS_ChanAimingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_ChanAimingSkill();

	virtual void ActiveSkill() override;
	virtual void OnSkillCanceledByDebuff() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void OnSkillCommand() override;
	virtual void InterruptSkill() override;

	// 스테미나 관리
	void TickDrainStamina();
	float GetCurrentStamina();

protected:
	// 스탠스 관리
	void StartHoldUp();

	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	virtual void DeactiveSkill() override;

	// 공격
	void OnShieldSlam();

	// 스테미나 관리
	float MaxStamina = 20.0f;
	float CurrentStamina;
	float StaminaDrainRate = 1.0f; // 초당 소비량
	float SlamStaminaCost = 20.0f;
	FTimerHandle StaminaDrainHandle;

	// UI
	void UpdateProgressBar(float InStamina);
	void ShowProgressBar(bool bShow);

	FTimerHandle KnockbackHandle;

	//Range VFX
	FTimerHandle RangeVFXSpawnHandle;
	void SpawnAimingSkillVFX();
};
