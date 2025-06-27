// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresRollingSkill.h"
#include "Character/Player/Seeker/GS_Ares.h"

void UGS_AresRollingSkill::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}
	Super::ActiveSkill();
	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		if (!OwnerPlayer->GetSkillInputControl().CanInputRoll)
		{
			return;
		}
		
		if (OwnerPlayer->HasAuthority())
		{
			// 구르기 시작 사운드 재생
			const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
			if (SkillInfo && SkillInfo->SkillStartSound)
			{
				OwnerPlayer->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
			}

			OwnerPlayer->GetMesh()->GetAnimInstance()->StopAllMontages(0);
			OwnerPlayer->ComboInputClose();
			OwnerPlayer->CurrentComboIndex = 0;
			OwnerPlayer->SetSkillInputControl(false, false, false);
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

void UGS_AresRollingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
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
