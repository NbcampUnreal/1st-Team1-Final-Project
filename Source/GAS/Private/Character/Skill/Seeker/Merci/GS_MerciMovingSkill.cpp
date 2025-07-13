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
	if (!CanActiveInternally())
	{
		// 누른 시점에 쿨타임 중이었다면 무효 입력 플래그 설정
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

void UGS_MerciMovingSkill::OnSkillCommand()
{
	if (!CanActiveInternally() || bPressedDuringCooldown)
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
}

void UGS_MerciMovingSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter->GetSkillComp())
	{
		MerciCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Moving, false);
	}
}

bool UGS_MerciMovingSkill::CanActive() const
{
	return true;
}

bool UGS_MerciMovingSkill::CanActiveInternally() const
{
	return OwnerCharacter && !bIsCoolingDown;
}

