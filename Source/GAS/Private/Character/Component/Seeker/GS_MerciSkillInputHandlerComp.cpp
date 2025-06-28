// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/Seeker/GS_MerciSkillInputHandlerComp.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "EnhancedInputComponent.h"


UGS_MerciSkillInputHandlerComp::UGS_MerciSkillInputHandlerComp()
{

}

void UGS_MerciSkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	Super::OnRightClick(Instance);

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	
	if (!MerciCharacter->GetSkillInputControl().CanInputRC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Click Lock"));
		return;
	}


	if (MerciCharacter->IsDead())
	{
		return;
	}

	if (!bCtrlHeld)
	{
		MerciCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Aiming);
	}
	else
	{
		MerciCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Ultimate);
	}
}

void UGS_MerciSkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	Super::OnLeftClick(Instance);
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (!MerciCharacter->GetSkillInputControl().CanInputLC)
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
		
		if(MerciCharacter->ComboSkillDrawMontage)
		{
			MerciCharacter->DrawBow(MerciCharacter->ComboSkillDrawMontage);
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

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	
	if (!MerciCharacter->GetSkillInputControl().CanInputRoll || !MerciCharacter->GetSkillInputControl().CanInputRC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right Release Lock"));
		return;
	}
	
	if (MerciCharacter->IsDead())
	{
		return;
	}
	
	if (!bWasCtrlHeldWhenLeftClicked)
	{
		MerciCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Aiming);
	}
	else
	{
		
	}
}

void UGS_MerciSkillInputHandlerComp::OnLeftClickRelease(const FInputActionInstance& Instance)
{
	Super::OnLeftClickRelease(Instance);

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);

	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	if (!MerciCharacter->GetSkillInputControl().CanInputRoll || !MerciCharacter->GetSkillInputControl().CanInputLC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Left Release Lock"));
		return;
	}

	if (!bWasCtrlHeldWhenLeftClicked)
	{
		if (MerciCharacter->NormalArrowClass)
		{
			MerciCharacter->ReleaseArrow(MerciCharacter->NormalArrowClass);
		}
	}
	else
	{
		OwnerCharacter->GetSkillComp()->TrySkillCommand(ESkillSlot::Moving);
	}
}

void UGS_MerciSkillInputHandlerComp::OnRoll(const struct FInputActionInstance& Instance)
{
	Super::OnRoll(Instance);
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (!MerciCharacter->GetSkillInputControl().CanInputRoll)
	{
		UE_LOG(LogTemp, Warning, TEXT("Roll Lock"));
		return;
	}

	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	Super::OnRoll(Instance);
	
	if (MerciCharacter)
	{
		MerciCharacter->GetSkillComp()->TryActivateSkill(ESkillSlot::Rolling);
	}
}

void UGS_MerciSkillInputHandlerComp::OnScroll(const FInputActionInstance& Instance)
{
	if (OwnerCharacter->IsDead())
	{
		return;
	}

	float ScrollValue = Instance.GetValue().Get<float>();
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (ScrollValue > 0.f)
	{
		MerciCharacter->Server_ChangeArrowType(+1);
	}
	else if (ScrollValue < 0.f)
	{
		MerciCharacter->Server_ChangeArrowType(-1);
	}
}
