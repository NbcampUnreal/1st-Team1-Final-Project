// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanRollingSkill.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/GS_TpsController.h"
#include "Components/CapsuleComponent.h"

UGS_ChanRollingSkill::UGS_ChanRollingSkill()
{
	CurrentSkillType = ESkillSlot::Rolling;
}

void UGS_ChanRollingSkill::ActiveSkill()
{
	Super::ActiveSkill();
	StartCoolDown();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// 스킬 시작 사운드 재생
		// 구르기 시작 사운드 재생
		PlaySkillStartSound();
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
		OwnerPlayer->CanChangeSeekerGait = false;
		
		const FName RollDirection = CalRollDirection();
		if (RollDirection == FName("00"))
		{
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0], FName("F0"));
		}
		else
		{
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0], RollDirection);
		}

		OwnerPlayer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
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
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		OwnerPlayer->CanChangeSeekerGait = true;

		SetIsActive(false);

		OwnerPlayer->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
}

void UGS_ChanRollingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	if (AGS_Chan* AresCharacter = Cast<AGS_Chan>(OwnerCharacter))
	{
		if (AresCharacter->GetSkillComp())
		{
			AresCharacter->Multicast_SetIsFullBodySlot(false);
			AresCharacter->SetMoveControlValue(true, true);
			SetIsActive(false);
		}
	}
}
