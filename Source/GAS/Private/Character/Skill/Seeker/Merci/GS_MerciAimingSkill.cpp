// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciAimingSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"

UGS_MerciAimingSkill::UGS_MerciAimingSkill()
{
	static ConstructorHelpers::FClassFinder<AGS_SeekerMerciArrow> ArrowBP(TEXT("/Game/Weapons/Blueprints/BP_SeekerMerciArrowNormal"));
	if (ArrowBP.Succeeded())
	{
		ArrowClass = ArrowBP.Class;
	}
}

void UGS_MerciAimingSkill::ActiveSkill()
{
	if (!CanActiveInternally())
	{
		UE_LOG(LogTemp, Error, TEXT("Can't Active Skill!!!!!!!!"));
		bPressedDuringCooldown = true;
		return;
	}
	//Super::ActiveSkill();
	// 유효 입력이므로 무효 입력 플래그 해제
	bPressedDuringCooldown = false;
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	MerciCharacter->SetDrawState(false);
	MerciCharacter->LeftClickPressedAttack(SkillAnimMontages[0]);
}

void UGS_MerciAimingSkill::OnSkillCommand()
{
	if (!CanActiveInternally() || bPressedDuringCooldown)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't Active Command!!!!!!!!"));
		return;
	}
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	MerciCharacter->LeftClickReleaseAttack(ArrowClass, 15.0f, 4);

	StartCoolDown();
}

bool UGS_MerciAimingSkill::CanActive() const
{
	return true;
}

bool UGS_MerciAimingSkill::CanActiveInternally() const
{
	return OwnerCharacter && !bIsCoolingDown;
}
