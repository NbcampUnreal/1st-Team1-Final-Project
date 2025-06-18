// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
//#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Ares.h"

void UGS_AresSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);

	AGS_Ares* Ares = Cast<AGS_Ares>(OwnerCharacter);

	if (!(Ares->GetSkillInputControl().CanInputRC))
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Click Lock"));
		return;
	}

	if (!bCtrlHeld)
	{
		if (!Ares->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
		{
			Ares->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}
	}
	else
	{
		Ares->GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
	}
}

void UGS_AresSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	AGS_Ares* Ares = Cast<AGS_Ares>(OwnerCharacter);

	if (!(Ares->GetSkillInputControl().CanInputLC))
	{
		return;
	}

	if (!bCtrlHeld)
	{
		if (Ares)
		{
			if (OwnerCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
			{
				OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
			}
			else
			{
				Ares->OnComboAttack();
			}
		}
	}
	else
	{
		if (Ares)
		{
			OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
		}
	}
}
