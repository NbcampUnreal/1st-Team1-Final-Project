// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillBase.h"
#include "Character/GS_Character.h"

float UGS_SkillBase::GetCoolTime()
{
	return Cooltime;
}

void UGS_SkillBase::InitSkill(AGS_Character* InOwner)
{
	OwnerCharacter = InOwner;
}

void UGS_SkillBase::ActiveSkill()
{
	if (!CanActive()) return;

	if (SkillAnimMontage)
	{
		OwnerCharacter->PlayAnimMontage(SkillAnimMontage);
	}

	StartCoolDown();
}

void UGS_SkillBase::ExecuteSkillEffect()
{
	// Override
}

void UGS_SkillBase::OnSkillCommand()
{
}

bool UGS_SkillBase::CanActive() const
{
	UE_LOG(LogTemp, Warning, TEXT("CanActive() - Character = %s"), OwnerCharacter ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("CanActive() - !bIsCoolingDown = %s"), !bIsCoolingDown ? TEXT("true") : TEXT("false"));
	return OwnerCharacter && !bIsCoolingDown;
}

bool UGS_SkillBase::IsActive() const
{
	return bIsActive;
}

void UGS_SkillBase::StartCoolDown()
{
	bIsCoolingDown = true;
	OwnerCharacter->GetWorldTimerManager().SetTimer(CooldownHandle, [this]()
		{
			bIsCoolingDown = false;
			UE_LOG(LogTemp, Warning, TEXT("Cool Down Complete"));
		}, Cooltime, false);
}
