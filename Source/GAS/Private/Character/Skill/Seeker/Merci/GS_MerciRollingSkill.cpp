// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciRollingSkill.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Components/CapsuleComponent.h"

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
			// 스킬 시작 사운드 재생
			if (UGS_SeekerAudioComponent* AudioComp = MerciCharacter->SeekerAudioComponent)
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
			}

			MerciCharacter->SetDrawState(false);
			MerciCharacter->SetAimState(false);
			MerciCharacter->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
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

			MerciCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

void UGS_MerciRollingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter))
	{
		if (MerciCharacter->HasAuthority())
		{
			MerciCharacter->Multicast_StopSkillMontage(SkillAnimMontages[0]);
			MerciCharacter->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
			MerciCharacter->CanChangeSeekerGait = true;

			// SeekerAudioComponent를 통한 스킬 종료 사운드
			if (UGS_SeekerAudioComponent* AudioComp = MerciCharacter->SeekerAudioComponent)
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
			}

			SetIsActive(false);

			MerciCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}
	}
}

void UGS_MerciRollingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Merci* AresCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (AresCharacter->GetSkillComp())
	{
		AresCharacter->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		AresCharacter->SetMoveControlValue(true, true);
		SetIsActive(false);
	}
}
