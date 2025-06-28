// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciRollingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"

UGS_MerciRollingSkill::UGS_MerciRollingSkill()
{
	CurrentSkillType = ESkillSlot::Rolling;
}

void UGS_MerciRollingSkill::ActiveSkill()
{
	Super::ActiveSkill();
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter)
	{
		// 구르기 시작 사운드 재생
		const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
		if (SkillInfo && SkillInfo->SkillStartSound)
		{
			MerciCharacter->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
		}

		MerciCharacter->SetDrawState(false);
		MerciCharacter->SetAimState(false);
		MerciCharacter->Multicast_SetIsFullBodySlot(true);
		MerciCharacter->SetSkillInputControl(false, false, false);
		MerciCharacter->SetMoveControlValue(false, false);
		MerciCharacter->CanChangeSeekerGait = false;

		if (MerciCharacter->GetSkillComp())
		{
			MerciCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Rolling, true);
		}

		FName RollDirection = CalRollDirection();
		if (RollDirection == FName("00"))
		{
			MerciCharacter->Multicast_PlaySkillMontage(SkillAnimMontages[0], FName("F0"));
		}
		else
		{
			MerciCharacter->Multicast_PlaySkillMontage(SkillAnimMontages[0], RollDirection);
		}
	}
}

void UGS_MerciRollingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter)
	{
		MerciCharacter->Multicast_SetIsFullBodySlot(false);
		MerciCharacter->SetSkillInputControl(true, true, true);
		MerciCharacter->SetMoveControlValue(true, true);
		MerciCharacter->CanChangeSeekerGait = true;

		if (MerciCharacter->GetSkillComp())
		{
			MerciCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Rolling, false);
		}
	}
}


bool UGS_MerciRollingSkill::CanActive() const
{
	return Super::CanActive();
}
