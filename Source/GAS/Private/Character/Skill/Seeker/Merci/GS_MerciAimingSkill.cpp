// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciAimingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"

UGS_MerciAimingSkill::UGS_MerciAimingSkill()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_MerciAimingSkill::ActiveSkill()
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

		// 활 당기기
		MerciCharacter->DrawBow(SkillAnimMontages[0]);
	}
}

void UGS_MerciAimingSkill::OnSkillCommand()
{
	if (!CanActive() || !GetIsActive())
	{
		return;
	}

	// 활 놓기
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	bool IsFullyDrawn = MerciCharacter->GetIsFullyDrawn();
	if(MerciCharacter->NormalArrowClass)
	{
		MerciCharacter->ReleaseArrow(MerciCharacter->NormalArrowClass, 15.0f, 4);
	}
	if(IsFullyDrawn)
	{
		// 쿨타임 측정 시작
		StartCoolDown();
	}

	// 스킬 종료
	DeactiveSkill();
}

void UGS_MerciAimingSkill::OnSkillAnimationEnd()
{
}

void UGS_MerciAimingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	SetIsActive(false);
}

void UGS_MerciAimingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
}
