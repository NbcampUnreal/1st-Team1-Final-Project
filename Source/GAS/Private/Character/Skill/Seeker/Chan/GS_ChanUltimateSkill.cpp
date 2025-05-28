// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"
#include "Character/Player/GS_Player.h"

void UGS_ChanUltimateSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	AGS_Player* OwnerPlayer = Cast<AGS_Player>(OwnerCharacter);
	OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
}

void UGS_ChanUltimateSkill::ExecuteSkillEffect()
{
}
