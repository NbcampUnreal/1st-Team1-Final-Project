// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresRollingSkill.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Sound/GS_CharacterAudioComponent.h"

UGS_AresRollingSkill::UGS_AresRollingSkill()
{
	CurrentSkillType = ESkillSlot::Rolling;
}

void UGS_AresRollingSkill::ActiveSkill()
{	
	Super::ActiveSkill();

	StartCoolDown();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{		
		if (OwnerPlayer->HasAuthority())
		{
			// 스킬 시작 사운드 재생
			if (UGS_CharacterAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_CharacterAudioComponent>())
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
			}
			
			OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
			OwnerPlayer->SetMoveControlValue(false, false);
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

void UGS_AresRollingSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_AresRollingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->CanChangeSeekerGait = true;

		SetIsActive(false);
	}
}

void UGS_AresRollingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Ares* AresCharacter = Cast<AGS_Ares>(OwnerCharacter);
	if (AresCharacter->GetSkillComp())
	{
		AresCharacter->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		AresCharacter->CanChangeSeekerGait = true;
		SetIsActive(false);
	}
}
