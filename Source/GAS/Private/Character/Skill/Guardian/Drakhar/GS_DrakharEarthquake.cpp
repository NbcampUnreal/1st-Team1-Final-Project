// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Guardian/Drakhar/GS_DrakharEarthquake.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"

UGS_DrakharEarthquake::UGS_DrakharEarthquake()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_DrakharEarthquake::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}

	ExecuteSkillEffect();
}

void UGS_DrakharEarthquake::InterruptSkill()
{
	Super::InterruptSkill();
}

void UGS_DrakharEarthquake::ExecuteSkillEffect()
{
	bIsEarthquaking = true;
	
	StartCoolDown();

	OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
}
