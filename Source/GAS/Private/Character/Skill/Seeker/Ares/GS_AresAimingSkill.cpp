// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresAimingSkill.h"
#include "Character/Player/Seeker/GS_Ares.h"

UGS_AresAimingSkill::UGS_AresAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_AresAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
	}
}

void UGS_AresAimingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
}

void UGS_AresAimingSkill::OnSkillCommand()
{
	Super::OnSkillCommand();
}

void UGS_AresAimingSkill::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();
}

bool UGS_AresAimingSkill::IsActive() const
{
	return Super::IsActive();
}
