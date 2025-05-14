// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Character/Skill/Seeker/Chan/GS_ChanMovingSkill.h"
#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"


// Sets default values for this component's properties
UGS_SkillComp::UGS_SkillComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGS_SkillComp::TryActivateSkill(ESkillSlot Slot)
{
	UE_LOG(LogTemp, Warning, TEXT("Skill : Skill TryActiveSkill"));
	if (!bCanUseSkill)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		Server_TryActiveSkill(Slot);
		return;
	}

	if (SkillMap.Contains(Slot))
	{
		UGS_SkillBase* Skill = SkillMap[Slot];
		if (Skill && Skill->CanActive())
		{
			Skill->ActiveSkill();
		}
	}
}

void UGS_SkillComp::TrySkillCommand(ESkillSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_TrySkillCommand(Slot);
		return;
	}

	if (SkillMap.Contains(Slot))
	{
		if (UGS_SkillBase* Skill = SkillMap[Slot])
		{
			Skill->OnSkillCommand();
		}
	}
}

void UGS_SkillComp::SetSkill(ESkillSlot Slot, TSubclassOf<UGS_SkillBase> SkillClass)
{
	if (!SkillClass) return;

	UGS_SkillBase* Skill = NewObject<UGS_SkillBase>(this, SkillClass);
	if (!Skill) return;

	Skill->InitSkill(Cast<AGS_Character>(GetOwner()));
	SkillMap.Add(Slot, Skill);
}

void UGS_SkillComp::SetCanUseSkill(bool InCanUseSkill)
{
	bCanUseSkill = InCanUseSkill;
}

bool UGS_SkillComp::IsSkillActive(ESkillSlot Slot) const
{
	if (const UGS_SkillBase* const* SkillPtr = SkillMap.Find(Slot))
	{
		return (*SkillPtr && (*SkillPtr)->IsActive());
	}
	return false;
}


// Called when the game starts
void UGS_SkillComp::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);

	if (GetOwner()->HasAuthority())
	{
		InitSkills();
	}
}

void UGS_SkillComp::Server_TryActiveSkill_Implementation(ESkillSlot Slot)
{
	TryActivateSkill(Slot);
}

void UGS_SkillComp::Server_TrySkillCommand_Implementation(ESkillSlot Slot)
{
	TrySkillCommand(Slot);
}

void UGS_SkillComp::InitSkills()
{
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (!OwnerCharacter) return;

	// Test용 방패병 클래스 설정
	// Todo : 나중에는 클래스별로 다르게
	SetSkill(ESkillSlot::Aiming, UGS_ChanAimingSkill::StaticClass());
	SetSkill(ESkillSlot::Moving, UGS_ChanMovingSkill::StaticClass());
	SetSkill(ESkillSlot::Ultimate, UGS_ChanUltimateSkill::StaticClass());
}

