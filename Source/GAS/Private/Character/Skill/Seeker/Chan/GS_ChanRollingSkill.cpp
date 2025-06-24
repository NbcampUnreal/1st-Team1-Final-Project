// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanRollingSkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/GS_TpsController.h"

void UGS_ChanRollingSkill::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}
	Super::ActiveSkill();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (!OwnerPlayer->GetSkillInputControl().CanInputRoll)
		{
			return;
		}
		
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->GetMesh()->GetAnimInstance()->StopAllMontages(0);
			OwnerPlayer->ComboInputClose();
			OwnerPlayer->CurrentComboIndex = 0;
			OwnerPlayer->SetSkillInputControl(false, false, true);
			OwnerPlayer->Multicast_SetIsFullBodySlot(true);
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
			OwnerPlayer->CanChangeSeekerGait = false;
			FName RollDirection = CalRollDirection();
			if (RollDirection == FName("00"))
			{
				OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0], FName("F0"));
			}
			else
			{
				OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0], RollDirection);
			}
		}
	}
}

void UGS_ChanRollingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->Multicast_StopSkillMontage(SkillAnimMontages[0]);
			OwnerPlayer->ComboInputOpen();
			OwnerPlayer->SetSkillInputControl(true, true, true);
			OwnerPlayer->SetMoveControlValue(true, true);
			OwnerPlayer->Multicast_SetIsFullBodySlot(false);
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
		}
		OwnerPlayer->CanChangeSeekerGait = false;
	}
	
}