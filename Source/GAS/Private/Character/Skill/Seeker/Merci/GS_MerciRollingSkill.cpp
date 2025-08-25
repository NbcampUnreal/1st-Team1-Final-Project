// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciRollingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_MerciRollingSkill::UGS_MerciRollingSkill()
{
	CurrentSkillType = ESkillSlot::Rolling;
}

void UGS_MerciRollingSkill::ActiveSkill()
{
	Super::ActiveSkill();
	
	StartCoolDown();

	if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter))
	{
		if (MerciCharacter->HasAuthority())
		{
			// 스킬 시작 사운드 재생
			if (UGS_SeekerAudioComponent* AudioComp = MerciCharacter->SeekerAudioComponent)
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
			}

			MerciCharacter->SetDrawState(false);
			MerciCharacter->SetAimState(false);
			MerciCharacter->Multicast_SetIsFullBodySlot(true);
			MerciCharacter->SetSkillInputControl(false, false, false);
			MerciCharacter->SetMoveControlValue(false, false);
			MerciCharacter->CanChangeSeekerGait = false;

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
}

void UGS_MerciRollingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter))
	{
		if (MerciCharacter->HasAuthority())
		{
			MerciCharacter->Multicast_SetIsFullBodySlot(false);
			MerciCharacter->SetSkillInputControl(true, true, true);
			MerciCharacter->SetMoveControlValue(true, true);
			MerciCharacter->CanChangeSeekerGait = true;

			// SeekerAudioComponent를 통한 스킬 종료 사운드
			if (UGS_SeekerAudioComponent* AudioComp = MerciCharacter->SeekerAudioComponent)
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
			}

			SetIsActive(false);
		}
	}
}

void UGS_MerciRollingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Merci* AresCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (AresCharacter->GetSkillComp())
	{
		AresCharacter->Multicast_SetIsFullBodySlot(false);
		AresCharacter->SetMoveControlValue(true, true);
		SetIsActive(false);
	}
}
