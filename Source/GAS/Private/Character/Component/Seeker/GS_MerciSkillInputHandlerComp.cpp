// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_MerciSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"


UGS_MerciSkillInputHandlerComp::UGS_MerciSkillInputHandlerComp()
{

}

void UGS_MerciSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);

	if (!bCtrlHeld)
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
	}
	else
	{
		
	}
}

void UGS_MerciSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	Super::OnLeftClick(Instance);

	if (!bCtrlHeld)
	{
		AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
		if(MerciCharacter->ComboSkillDrawMontage)
		{
			MerciCharacter->LeftClickPressedAttack(MerciCharacter->ComboSkillDrawMontage);
		}
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Moving);
	}
}

void UGS_MerciSkillInputHandlerComp::OnRightClickRelease(const FInputActionInstance& Instance)
{
	Super::OnRightClickRelease(Instance);
	if (!bCtrlHeld)
	{
		OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
	}
	else
	{
		
	}
}

void UGS_MerciSkillInputHandlerComp::OnLeftClickRelease(const FInputActionInstance& Instance)
{
	Super::OnLeftClickRelease(Instance);
	if (!bCtrlHeld)
	{
		AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
		if (MerciCharacter->NormalArrowClass)
		{
			MerciCharacter->LeftClickReleaseAttack(MerciCharacter->NormalArrowClass);
		}
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Moving);
	}
}
