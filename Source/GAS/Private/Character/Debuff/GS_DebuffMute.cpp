// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffMute.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Skill/GS_SkillComp.h"

class AGS_Guardian;

void UGS_DebuffMute::OnApply()
{
	Super::OnApply();

	if (TargetCharacter)
	{
		AGS_Guardian *Guardian= Cast<AGS_Guardian>(TargetCharacter);
		Guardian->QuitGuardianSkill();
		
		// if (UGS_SkillComp* SkillComp = TargetCharacter->GetSkillComp())
		// {
		// 	SkillComp->Server_TryDeactiveSkill(ESkillSlot::Aiming);
		// 	SkillComp->Server_TryDeactiveSkill(ESkillSlot::Moving);
		// 	SkillComp->Server_TryDeactiveSkill(ESkillSlot::Ready);
		// }
	}
}

void UGS_DebuffMute::OnExpire()
{
	Super::OnExpire();
}
