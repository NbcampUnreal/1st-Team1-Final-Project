// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Guardian/Drakhar/GS_DrakharEarthquake.h"
#include "Character/Player/Guardian/GS_Drakhar.h"

UGS_DrakharEarthquake::UGS_DrakharEarthquake()
{
	Cooltime = 15.f;
	bIsEarthquaking = false;
	
	static ConstructorHelpers::FObjectFinder<UAnimMontage> EarthquakeMontage(TEXT("/Game/Player/Guardian/Drakhar/Animations/Blueprint/AM_Earthquake.AM_Earthquake"));
	if (EarthquakeMontage.Succeeded())
	{
		SkillAnimMontages.Add(EarthquakeMontage.Object);
	}
}

void UGS_DrakharEarthquake::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}
	ExecuteSkillEffect();
}

void UGS_DrakharEarthquake::ExecuteSkillEffect()
{
	bIsEarthquaking = true;
	
	StartCoolDown();

	OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
}
