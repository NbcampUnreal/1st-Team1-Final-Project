// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "Character/Component/GS_StatRow.h"
#include "GS_AresUltimateSkill.generated.h"

struct FGS_StatRow;

UCLASS()
class GAS_API UGS_AresUltimateSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	UGS_AresUltimateSkill();

	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
	virtual void ExecuteSkillEffect() override;
	virtual bool IsActive() const override;

private:
	bool bIsBerserker;

	FTimerHandle UltimateSkillTimerHandle;
	FGS_StatRow BuffAmount;

	// Cooltime 복원용 변수
	float OriginalMovingSkillCooltime = -1.f;
};
