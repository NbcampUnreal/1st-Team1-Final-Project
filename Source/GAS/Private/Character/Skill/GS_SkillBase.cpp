// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillBase.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"

UTexture2D* UGS_SkillBase::GetSkillImage()
{
	if (SkillImage)
	{
		return SkillImage;
	}
	return nullptr;
}

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

	StartCoolDown();
}

void UGS_SkillBase::DeactiveSkill()
{
}

void UGS_SkillBase::ExecuteSkillEffect()
{
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
	//server

	if (Cooltime <= 0.f)
	{
		bIsCoolingDown = false;
		return;
	}

	bIsCoolingDown = true;
	
	OwnerCharacter->GetWorldTimerManager().SetTimer(CooldownHandle, [this]()
		{
			bIsCoolingDown = false;
			UE_LOG(LogTemp, Warning, TEXT("Cool Down Complete"));

		
		}, Cooltime, false);

	GetWorld()->GetTimerManager().SetTimer(LogTimerHandle, this, &UGS_SkillBase::LogRemainingTime, 0.07f, true);
}

void UGS_SkillBase::LogRemainingTime()
{
	//server
	LeftCoolTime = GetWorld()->GetTimerManager().GetTimerRemaining(CooldownHandle);
	
	SetCoolTime(LeftCoolTime);
	
	if (LeftCoolTime <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LogTimerHandle);

		SetCoolTime(Cooltime);
	}
}

void UGS_SkillBase::SetCoolTime(float InCoolTime)
{
	if (CurrentSkillType==ESkillSlot::Moving)
	{
		OwnerCharacter->GetSkillComp()->Skill1LeftCoolTime = InCoolTime;
	}
	if (CurrentSkillType==ESkillSlot::Aiming)
	{
		OwnerCharacter->GetSkillComp()->Skill2LeftCoolTime = InCoolTime;
	}
	if (CurrentSkillType==ESkillSlot::Ultimate)
	{
		OwnerCharacter->GetSkillComp()->Skill3LeftCoolTime = InCoolTime;
	}
}