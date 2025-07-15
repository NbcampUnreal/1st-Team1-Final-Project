// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/GS_TpsController.h"

void UGS_SeekerSkillBase::ActiveSkill()
{
	Super::ActiveSkill();
}

void UGS_SeekerSkillBase::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_SeekerSkillBase::OnSkillAnimationEnd()
{
}

void UGS_SeekerSkillBase::ApplyEffectToGuardian(AGS_Guardian* Target)
{
}

void UGS_SeekerSkillBase::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
}

void UGS_SeekerSkillBase::ExecuteSkillEffect()
{

}

FName UGS_SeekerSkillBase::CalRollDirection()
{
	FString RollDirection = "";

	if (AGS_Seeker* OwnerPlayer = Cast<AGS_Seeker>(OwnerCharacter))
	{
		const FVector2D MoveInputValue = (Cast<AGS_TpsController>(OwnerPlayer->GetController()))->MoveInputValue;
		if (MoveInputValue.X > 0)
		{
			RollDirection += TEXT("F");
		}
		else if (MoveInputValue.X < 0)
		{
			RollDirection += TEXT("B");
		}
		else
		{
			RollDirection += TEXT("0");
		}

		if (MoveInputValue.Y > 0)
		{
			RollDirection += TEXT("R");
		}
		else if (MoveInputValue.Y < 0)
		{
			RollDirection += TEXT("L");
		}
		else
		{
			RollDirection += TEXT("0");
		}
	}
	return FName(*RollDirection);
}
