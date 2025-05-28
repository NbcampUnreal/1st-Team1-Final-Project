// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_ChanSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);
	if (!bCtrlHeld)
	{
		if (OwnerCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
		{
			OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
		}
		else
		{
			OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
	}
}

void UGS_ChanSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	if (!bCtrlHeld)
	{
		AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwnerCharacter);
		if (ChanCharacter)
		{
			ChanCharacter->LeftClickPressed_Implementation();
		}
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
	}
}

