// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/Seeker/GS_Ares.h"

void UGS_AresSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);
	UE_LOG(LogTemp, Warning, TEXT("Right Click Ares"));
	if (OwnerCharacter->IsDead())
	{
		return;
	}

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
			Ares->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Aiming);
		}
	}
	else
	{
		Ares->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Ultimate);
	}
}

void UGS_AresSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	Super::OnLeftClick(Instance);
	AGS_Ares* Ares = Cast<AGS_Ares>(OwnerCharacter);
	
	if (!(Ares->GetSkillInputControl().CanInputLC))
	{
		UE_LOG(LogTemp, Warning, TEXT("Left Click Lock"));
		return;
	}
	
	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	if (!bCtrlHeld)
	{
		if (Ares)
		{
			if (Ares->GetSkillComp()->IsSkillActive(ESkillSlot::Aiming))
			{
				Ares->GetSkillComp()->Server_TrySkillCommand(ESkillSlot::Aiming);
			}
			else
			{				
				if (Ares->CanAcceptComboInput)
				{
					Ares->Server_OnComboAttack();
				}
				else
				{
					
				}
			}
		}
	}
	else
	{
		if (Ares)
		{
			Ares->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Moving);
		}
	}
}

void UGS_AresSkillInputHandlerComp::OnLeftClickRelease(const FInputActionInstance& Instance)
{
	Super::OnLeftClickRelease(Instance);
	
	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	if (bWasCtrlHeldWhenLeftClicked && OwnerCharacter->GetSkillInputControl().CanInputCtrl)
	{
		OwnerCharacter->GetSkillComp()->Server_TrySkillCommand(ESkillSlot::Moving);
	}
}

void UGS_AresSkillInputHandlerComp::OnRoll(const struct FInputActionInstance& Instance)
{
	AGS_Ares* Ares = Cast<AGS_Ares>(OwnerCharacter);
	if (!Ares->GetSkillInputControl().CanInputRoll)
	{
		UE_LOG(LogTemp, Warning, TEXT("Roll Lock"));
		return;
	}
	
	if (Ares->IsDead())
	{
		return;
	}
	if (Ares)
	{
		Ares->GetSkillComp()->Server_TryActivateSkill(ESkillSlot::Rolling);
	}

	return;
}

void UGS_AresSkillInputHandlerComp::OnKeyReset(const struct FInputActionInstance& Instance)
{
	Super::OnKeyReset(Instance);
}
