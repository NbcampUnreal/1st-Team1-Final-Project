// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresUltimateSkill.h"

void UGS_AresUltimateSkill::ActiveSkill()
{
	Super::ActiveSkill();

	bIsBerserker = true;
	ExecuteSkillEffect();
}

void UGS_AresUltimateSkill::DeactiveSkill()
{
}

void UGS_AresUltimateSkill::ExecuteSkillEffect()
{
	// 데미지 무효화
	// 공격력 증가
	// 공격 속도 증가
	// AresMovingSkill 쿨타임 감소
}

bool UGS_AresUltimateSkill::IsActive() const
{
	return bIsBerserker;
}
