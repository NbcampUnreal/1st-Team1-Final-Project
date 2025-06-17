// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_ChanSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);

	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwnerCharacter);

	if (!ChanCharacter->GetSkillInputControl().CanInputRC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Click Lock"));
		return;
	}
	
	if (!bCtrlHeld)
	{
		if (!ChanCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
		{
			ChanCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}
	}
	else
	{
		ChanCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
	}
}

void UGS_ChanSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwnerCharacter);

	if (!ChanCharacter->GetSkillInputControl().CanInputLC)
	{
		return;
	}
	
	if (!bCtrlHeld)
	{
		if (ChanCharacter)
		{
			if (ChanCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
			{
				ChanCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
			}
			else
			{
				
				if (ChanCharacter->CanAcceptComboInput)
				{
					ChanCharacter->OnComboAttack();
				}
				else
				{
				}
			}
		}
	}
	else
	{
		if (ChanCharacter)
		{
			ChanCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
		}
	}
}

