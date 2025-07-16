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
		// 구르기 시작 사운드 재생
		const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
		if (SkillInfo && SkillInfo->SkillStartSound)
		{
			OwnerPlayer->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
		}

			OwnerPlayer->Multicast_SetIsFullBodySlot(true);
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
			OwnerPlayer->SetSkillInputControl(false, false, false);
			OwnerPlayer->SetMoveControlValue(false, false);
			OwnerPlayer->CanChangeSeekerGait = false;
			if (OwnerCharacter->GetSkillComp())
			{
				OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Rolling, true);
			}
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

void UGS_ChanRollingSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();

	
}

void UGS_ChanRollingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_StopSkillMontage(SkillAnimMontages[0]);
		OwnerPlayer->Multicast_SetIsFullBodySlot(false);
		OwnerPlayer->SetSkillInputControl(true, true, true);
		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->CanChangeSeekerGait = true;

		if (OwnerCharacter->GetSkillComp())
		{
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Rolling, false);
		}

	}
}

void UGS_ChanRollingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Chan* AresCharacter = Cast<AGS_Chan>(OwnerCharacter);
	if (AresCharacter->GetSkillComp())
	{
		AresCharacter->Multicast_SetIsFullBodySlot(false);
		AresCharacter->SetMoveControlValue(true, true);
		AresCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Rolling, false);
	}
}
