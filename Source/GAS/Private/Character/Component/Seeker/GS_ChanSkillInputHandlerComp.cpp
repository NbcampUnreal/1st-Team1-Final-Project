// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"

void UGS_ChanSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);
	if (OwnerCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
	{
		OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
	}
}

void UGS_ChanSkillInputHandlerComp::OnCtrlLeftClick(const FInputActionInstance& Instance)
{
	Super::OnCtrlLeftClick(Instance);
	OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
}

void UGS_ChanSkillInputHandlerComp::OnCtrlRightClick(const FInputActionInstance& Instance)
{
	Super::OnCtrlRightClick(Instance);
	OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
}
