// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_MerciUltimateSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_MerciUltimateSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_MerciUltimateSkill();
	
	virtual void ActiveSkill() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void InterruptSkill() override;

private:
	virtual void DeactiveSkill() override;

	// 스탠스 관리
	FTimerHandle AutoAimingHandle;
	FTimerHandle AutoAimTickHandle;
	float AutoAimTickInterval = 0.2f;

	void AutoAimingStart();
	float AutoAimingStateTime = 20.0f;

	AActor* FindCloseTarget();
	UPROPERTY()
	TArray<AActor*> AllMonsterActors;

	void TickAutoAimTarget();
	void UpdateMonsterList();

	AActor* CurrentTarget;
};
