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

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Montage(TEXT("/Game/Player/Seeker/Merci/Animation/AnimSequence/WithBow/StandingDrawArrow/AM_MerciDraw"));
	if (Montage.Succeeded())
	{
		SkillAnimMontages.Add(Montage.Object);
		UE_LOG(LogTemp, Warning, TEXT("Succeeded"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Montage not Succeeded"));
	}
}

void UGS_MerciMovingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	MerciCharacter->SetDrawState(false);
	UE_LOG(LogTemp, Warning, TEXT("ActiveSkill : %s"), *SkillAnimMontages[0]->GetName());
	MerciCharacter->LeftClickPressedAttack(SkillAnimMontages[0]);
}

void UGS_MerciMovingSkill::OnSkillCommand()
{
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	MerciCharacter->LeftClickReleaseAttack(ArrowClass);
}
