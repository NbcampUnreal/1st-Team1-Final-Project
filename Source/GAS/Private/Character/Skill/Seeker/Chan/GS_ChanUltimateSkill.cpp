// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "AkAudioEvent.h"

UGS_ChanUltimateSkill::UGS_ChanUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}

void UGS_ChanUltimateSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	
	// 궁극기 사운드 재생
	if (OwnerPlayer && OwnerPlayer->UltimateSkillSound)
	{
		OwnerPlayer->Multicast_PlaySkillSound(OwnerPlayer->UltimateSkillSound);
	}
	
	OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
}

void UGS_ChanUltimateSkill::ExecuteSkillEffect()
{
}
