// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffStun.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"

void UGS_DebuffStun::OnApply()
{
	Super::OnApply();

	if (TargetCharacter)
	{
		// 움직임도 멈춤
		TargetCharacter->GetCharacterMovement()->DisableMovement();
		
		// 스킬 못쓰고
		// 스킬을 끊지는 않음
		if (UGS_SkillComp* SkillComp = TargetCharacter->FindComponentByClass<UGS_SkillComp>())
		{
			SkillComp->SetCanUseSkill(false);
		}
	}
}

void UGS_DebuffStun::OnExpire()
{
	if (TargetCharacter)
	{
		// 움직임
		TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		// 스킬 사용 가능
		if (UGS_SkillComp* SkillComp = TargetCharacter->FindComponentByClass<UGS_SkillComp>())
		{
			SkillComp->SetCanUseSkill(true);
		}
	}

	Super::OnExpire();
}
