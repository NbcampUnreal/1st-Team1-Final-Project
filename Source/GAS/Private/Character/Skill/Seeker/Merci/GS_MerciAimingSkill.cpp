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
	if (!CanActiveInternally())
	{
		bPressedDuringCooldown = true;
		return;
	}
	
	// 유효 입력이므로 무효 입력 플래그 해제
	bPressedDuringCooldown = false;
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

void UGS_MerciAimingSkill::OnSkillCommand()
{
	if (!CanActiveInternally() || bPressedDuringCooldown)
	{
		return;
	}

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	bool IsFullyDrawn = MerciCharacter->GetIsFullyDrawn();
	if(MerciCharacter->NormalArrowClass)
	{
		MerciCharacter->ReleaseArrow(MerciCharacter->NormalArrowClass, 15.0f, 4);
	}
	if(IsFullyDrawn)
	{
		StartCoolDown();
	}
}

void UGS_MerciAimingSkill::OnSkillAnimationEnd()
{
}

void UGS_MerciAimingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter->GetSkillComp())
	{
		MerciCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Aiming, false);
	}
}

bool UGS_MerciAimingSkill::CanActive() const
{
	return true;
}

bool UGS_MerciAimingSkill::CanActiveInternally() const
{
	return OwnerCharacter && !bIsCoolingDown;
}
