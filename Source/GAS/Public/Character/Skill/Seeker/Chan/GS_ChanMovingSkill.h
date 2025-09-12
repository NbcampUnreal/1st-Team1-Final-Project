// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "Character/Component/GS_StatRow.h"
#include "GS_ChanMovingSkill.generated.h"

struct FGS_StatRow;

UCLASS()
class GAS_API UGS_ChanMovingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_ChanMovingSkill();

	virtual void ActiveSkill() override;
	virtual void OnSkillCanceledByDebuff() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void InterruptSkill() override;

protected:
	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	virtual void DeactiveSkill() override;
	void AggroToOwner();
	void StrengthenDefense();

	FTimerHandle DEFBuffHandle;
	float StrengthenDefenseDuration = 20.0f;
	float ExtraDefense = 200.0f;
	void DeactiveDEFBuff();
	FGS_StatRow BuffAmount;

};
