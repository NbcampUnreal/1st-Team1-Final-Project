// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanRollingSkill.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/GS_TpsController.h"
#include "Components/CapsuleComponent.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_ChanRollingSkill::UGS_ChanRollingSkill()
{
}

void UGS_ChanRollingSkill::ActiveSkill()
{
	Super::ActiveSkill();
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
		if (OwnerPlayer->HasAuthority())
		{
			OwnerPlayer->Multicast_StopSkillMontage(SkillAnimMontages[0]);
			OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
			OwnerPlayer->CanChangeSeekerGait = true;

			// 스킬 종료 사운드 재생 (멀티캐스트)
			if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->SeekerAudioComponent)
			{
				AudioComp->RequestSkillAudio(CurrentSkillType, 1);
			}

			SetIsActive(false);

			OwnerPlayer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}
	}
}

void UGS_ChanRollingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	if (AGS_Chan* AresCharacter = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (AresCharacter->GetSkillComp())
		{
			AresCharacter->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
			AresCharacter->SetMoveControlValue(true, true);
			SetIsActive(false);
		}
	}
}
