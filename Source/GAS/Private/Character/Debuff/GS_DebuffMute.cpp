// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffMute.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"

void UGS_DebuffMute::OnApply()
{
	Super::OnApply();

	if (TargetCharacter)
	{
		if (UGS_SkillComp* SkillComp = TargetCharacter->GetSkillComp())
		{
			SkillComp->Server_TryDeactiveSkill(ESkillSlot::Aiming);
			SkillComp->Server_TryDeactiveSkill(ESkillSlot::Moving);
			SkillComp->Server_TryDeactiveSkill(ESkillSlot::Ready);
		}
	}
}

void UGS_DebuffMute::OnExpire()
{
}
