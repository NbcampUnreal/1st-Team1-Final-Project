// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciMovingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"

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
		const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
		if (SkillInfo && SkillInfo->SkillStartSound)
		{
			MerciCharacter->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
		}
		
		MerciCharacter->SetDrawState(false);
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

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	bool IsFullyDrawn = MerciCharacter->GetIsFullyDrawn();

	if (MerciCharacter->SmokeArrowClass)
	{
		MerciCharacter->ReleaseArrow(MerciCharacter->SmokeArrowClass);
	}
	
	if (IsFullyDrawn)
	{
		StartCoolDown();
	}

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
	Super::DeactiveSkill();
}

