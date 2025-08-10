// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_ChanSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Seeker/GS_Chan.h"

void UGS_ChanSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwnerCharacter);
	/*if (!ChanCharacter->GetSkillInputControl().CanInputRC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Click Lock"));
		return;
	}*/

	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	Super::OnRightClick(Instance);

	if (!bCtrlHeld)
	{
		ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Aiming);
		/*if (!ChanCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming)) // 해당 Skill 이 Stat 에 없거나 bIsActive 가 false 라면 
		{
			ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Aiming);
		}*/
	}
	else
	{
		ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Ultimate);
	}
	
	/*if (!bCtrlHeld)
	{
		if (!ChanCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming)) // 해당 Skill 이 Stat 에 없거나 bIsActive 가 false 라면 
		{
			ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Aiming);
		}
	}
	else
	{
		ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Ultimate);
	}*/
}

void UGS_ChanSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwnerCharacter);

	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	Super::OnLeftClick(Instance);
	
	if (!bCtrlHeld)
	{
		if (ChanCharacter)
		{
			if (ChanCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
			{
				ChanCharacter->GetSkillComp()->Server_TrySkillCommand(ESkillSlot::Aiming);
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
			ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Moving);
		}
	}
}

void UGS_ChanSkillInputHandlerComp::OnRoll(const struct FInputActionInstance& Instance)
{
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwnerCharacter);
	
	Super::OnRoll(Instance);
	
	if (ChanCharacter)
	{
		ChanCharacter->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Rolling);
	}

	return;
}

void UGS_ChanSkillInputHandlerComp::OnKeyReset(const struct FInputActionInstance& Instance)
{
	Super::OnKeyReset(Instance);
}

