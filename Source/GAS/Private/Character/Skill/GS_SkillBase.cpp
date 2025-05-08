// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillBase.h"
#include "Character/GS_Character.h"

float UGS_SkillBase::GetCoolTime()
{
	return Cooltime;
}

void UGS_SkillBase::InitSkill(AGS_Character* InOwner)
{
	OwnerCharacter = InOwner;
}

void UGS_SkillBase::ActiveSkill()
{
	if (!CanActive()) return;

	if (SkillAnimMontage)
	{
		OwnerCharacter->PlayAnimMontage(SkillAnimMontage);
	}

	StartCoolDown();
}

void UGS_SkillBase::ExecuteSkillEffect()
{
	// Override
}

bool UGS_SkillBase::CanActive() const
{
	return OwnerCharacter && !bIsCoolingDown;
}

void UGS_SkillBase::StartCoolDown()
{
	bIsCoolingDown = true;
	OwnerCharacter->GetWorldTimerManager().SetTimer(CooldownHandle, [this]()
		{
			bIsCoolingDown = false;
		}, Cooltime, false);
}
