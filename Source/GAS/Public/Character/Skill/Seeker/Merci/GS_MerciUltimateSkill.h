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
	virtual void ExecuteSkillEffect() override;
	virtual void InterruptSkill() override;
	virtual bool IsActive() const override;

private:
	// 스탠스 관리
	bool bIsAutoAimingState;
	FTimerHandle AutoAimingHandle;
	FTimerHandle AutoAimTickHandle;
	float AutoAimTickInterval = 0.2f;

	void DeActiveAutoAimingState();
	float AutoAimingStateTime = 20.0f;

	AActor* FindCloseTarget();
	UPROPERTY()
	TArray<AActor*> AllMonsterActors;

	void TickAutoAimTarget();
	void UpdateMonsterList();

	AActor* CurrentTarget;
};
