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
	if (!bCanUseSkill)
	{
		return;
	}

	if (GetOwnerRole() < ROLE_Authority)
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


// Called when the game starts
void UGS_SkillComp::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);
	InitSkills();
}

void UGS_SkillComp::Server_TryActiveSkill_Implementation(ESkillSlot Slot)
{
	TryActivateSkill(Slot);
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

