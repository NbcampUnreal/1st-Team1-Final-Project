// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciMovingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_MerciMovingSkill::UGS_MerciMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_MerciMovingSkill::ActiveSkill()
{
	Super::ActiveSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter)
	{
		// 스킬 시작 사운드 재생
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
		{
			if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
			}
		}
		
		MerciCharacter->SetDrawState(false);

		// 활 당기기
		MerciCharacter->DrawBow(SkillAnimMontages[0]);
	}
}

void UGS_MerciMovingSkill::OnSkillAnimationEnd()
{
}

void UGS_MerciMovingSkill::OnSkillCommand()
{
	if (!CanActive() || !GetIsActive())
	{
		return;
	}

	// 활 놓기
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	bool IsFullyDrawn = MerciCharacter->GetIsFullyDrawn();

	if (MerciCharacter->SmokeArrowClass)
	{
		MerciCharacter->ReleaseArrow(MerciCharacter->SmokeArrowClass);
	}
	
	if (IsFullyDrawn)
	{
		// 쿨타임 측정 시작
		StartCoolDown();
	}

	// 스킬 종료
	DeactiveSkill();
}

void UGS_MerciMovingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	SetIsActive(false);
}

void UGS_MerciMovingSkill::DeactiveSkill()
{
	// SeekerAudioComponent를 통한 스킬 종료 사운드
	if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
		}
	}

	Super::DeactiveSkill();
}

