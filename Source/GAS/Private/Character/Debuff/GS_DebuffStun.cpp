// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffStun.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"

void UGS_DebuffStun::OnApply()
{
	Super::OnApply();
	if (!TargetCharacter->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Is Not Server OnApply for %s"), *TargetCharacter->GetName());
	}

	if (TargetCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Apply Stun Debuff for %s"), *TargetCharacter->GetName());
		// 움직임도 멈춤
		TargetCharacter->GetCharacterMovement()->DisableMovement();
		// 스킬 못쓰고
		// 스킬을 끊지는 않음
		if (UGS_SkillComp* SkillComp = TargetCharacter->GetSkillComp())
		{
			SkillComp->SetCanUseSkill(false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Apply Stun Debuff Error - TargetCharacter is null"));
	}
}

void UGS_DebuffStun::OnExpire()
{
	if (!TargetCharacter->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Is Not Server OnExpiree for %s"), *TargetCharacter->GetName());
	}

	if (TargetCharacter)
	{
		// 움직임
		TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		// 스킬 사용 가능
		if (UGS_SkillComp* SkillComp = TargetCharacter->GetSkillComp())
		{
			SkillComp->SetCanUseSkill(true);
		}
		UE_LOG(LogTemp, Warning, TEXT("Stun expired for %s"), *TargetCharacter->GetName());
	}

	Super::OnExpire();
}
