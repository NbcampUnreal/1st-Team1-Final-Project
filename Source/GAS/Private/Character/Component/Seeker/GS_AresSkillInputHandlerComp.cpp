// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Ares.h"

void UGS_AresSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);
	UE_LOG(LogTemp, Warning, TEXT("Right Click Ares"));
	if (OwnerCharacter->IsDead())
	{
		return;
	}

	if (!Cast<AGS_Player>(OwnerCharacter)->GetSkillInputControl().CanInputRC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Click Lock"));
		return;
	}

	if (!bCtrlHeld)
	{
		if (!OwnerCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
		{
			OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
		}
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
	}
}

void UGS_AresSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	Super::OnLeftClick(Instance);
	UE_LOG(LogTemp, Warning, TEXT("Left Click Ares"));
	if (OwnerCharacter->IsDead())
	{
		return;
	}

	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);

	if (!Cast<AGS_Player>(OwnerCharacter)->GetSkillInputControl().CanInputLC)
	{
		return;
	}

	if (!bCtrlHeld)
	{
		if (AresCharacter)
		{
			if (OwnerCharacter->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
			{
				OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
			}
			else
			{

			}
		}
	}
	else
	{
		if (AresCharacter)
		{
			OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
		}
	}
}
