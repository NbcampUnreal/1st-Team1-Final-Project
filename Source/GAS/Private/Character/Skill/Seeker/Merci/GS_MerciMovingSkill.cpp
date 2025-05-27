// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciMovingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"

UGS_MerciMovingSkill::UGS_MerciMovingSkill()
{
	static ConstructorHelpers::FClassFinder<AGS_SeekerMerciArrow> ArrowBP(TEXT("/Game/Weapons/Blueprints/BP_SeekerMerciArrowSmoke"));
	if (ArrowBP.Succeeded())
	{
		ArrowClass = ArrowBP.Class;
	}
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
	MerciCharacter->SetDrawState(false);
	MerciCharacter->LeftClickPressedAttack(SkillAnimMontages[0]);
}

void UGS_MerciMovingSkill::OnSkillCommand()
{
	if (!CanActiveInternally() || bPressedDuringCooldown)
	{
		return;
	}
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	bool IsFullyDrawn = MerciCharacter->GetIsFullyDrawn();

	MerciCharacter->LeftClickReleaseAttack(ArrowClass);
	if (IsFullyDrawn)
	{
		StartCoolDown();
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

