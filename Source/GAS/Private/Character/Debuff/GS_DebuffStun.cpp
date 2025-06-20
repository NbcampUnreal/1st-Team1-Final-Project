// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Debuff/GS_DebuffStun.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_TpsController.h"
#include "AI/GS_AIController.h"

void UGS_DebuffStun::OnApply()
{
	Super::OnApply();
	if (!TargetCharacter->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Is Not Server OnApply for %s"), *TargetCharacter->GetName());
	}

	if (TargetCharacter)
	{
		// 움직임도 멈춤(가디언)
		if(AGS_TpsController* Controller = Cast<AGS_TpsController>(TargetCharacter->GetController()))
		{
			Controller->SetMoveControlValue(false, false);
		}
		else if(AGS_AIController* AI = Cast<AGS_AIController>(TargetCharacter->GetController()))
		{
			MaxSpeed = TargetCharacter->GetCharacterMovement()->MaxWalkSpeed;
			TargetCharacter->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		}
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
		// 움직임(가디언
		if(AGS_TpsController* Controller = Cast<AGS_TpsController>(TargetCharacter->GetController()))
		{
			Controller->SetMoveControlValue(true, true);
		}
		else
		{
			TargetCharacter->GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
		}

		// 스킬 사용 가능
		if (UGS_SkillComp* SkillComp = TargetCharacter->GetSkillComp())
		{
			SkillComp->SetCanUseSkill(true);
		}
		UE_LOG(LogTemp, Warning, TEXT("Stun expired for %s"), *TargetCharacter->GetName());
	}

	Super::OnExpire();
}
