// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_MerciSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"


void UGS_MerciSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);

	if (!bCtrlHeld)
	{
		
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
	}
}

void UGS_MerciSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
}

void UGS_MerciSkillInputHandlerComp::OnRightClickRelease(const FInputActionInstance& Instance)
{
	if (!bCtrlHeld)
	{

	}
	else
	{
		OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
	}
}

void UGS_MerciSkillInputHandlerComp::OnLeftClickRelease(const FInputActionInstance& Instance)
{
}
