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

	// 돌진 시작
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &UGS_ChanUltimateSkill::StartCharge, 0.5f, false);
}

void UGS_ChanUltimateSkill::ExecuteSkillEffect()
{

}

void UGS_ChanUltimateSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
}

void UGS_ChanUltimateSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
}

void UGS_ChanUltimateSkill::StartCharge()
{
}

void UGS_ChanUltimateSkill::UpdateCharge()
{
}

void UGS_ChanUltimateSkill::EndCharge()
{
}

void UGS_ChanUltimateSkill::CheckAndApplyDamage()
{
}

bool UGS_ChanUltimateSkill::IsEnemyTeam(AGS_Player* Player1, AGS_Player* Player2)
{
	return false;
}

void UGS_ChanUltimateSkill::ApplyDamageToTarget(AGS_Player* Target, float DamageAmount)
{
}
