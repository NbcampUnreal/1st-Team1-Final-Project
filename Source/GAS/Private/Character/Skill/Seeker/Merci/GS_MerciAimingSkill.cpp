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
		UE_LOG(LogTemp, Warning, TEXT("ArrowClass loaded: %s"), *ArrowClass->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ArrowClass load failed!"));
	}
}

void UGS_MerciAimingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	MerciCharacter->SetDrawState(false);
	UE_LOG(LogTemp, Warning, TEXT("ActiveSkill : %s"), *SkillAnimMontages[0]->GetName());
	MerciCharacter->LeftClickPressedAttack(SkillAnimMontages[0]);
}

void UGS_MerciAimingSkill::OnSkillCommand()
{
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	MerciCharacter->LeftClickReleaseAttack(ArrowClass, 25.0f, 4);
}
