// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresUltimateSkill.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_StatRow.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Seeker.h"

UGS_AresUltimateSkill::UGS_AresUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}


void UGS_AresUltimateSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	UE_LOG(LogTemp, Warning, TEXT("Ares Ultimate Skill Active"));

	// 스킬 시작 사운드 재생
	const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
	if (AGS_Seeker* OwnerPlayer = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (SkillInfo && SkillInfo->SkillStartSound)
		{
			OwnerPlayer->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
		}

		OwnerPlayer->Multicast_SetIsFullBodySlot(true);
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		OwnerPlayer->SetSkillInputControl(false, false, false);
		OwnerPlayer->SetMoveControlValue(false, false);
	}
	
	bIsBerserker = true;
	// 만약 일정 시간 후 효과 해제를 원하면, 타이머로 DeactiveSkill 호출
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(UltimateSkillTimerHandle, this, &UGS_AresUltimateSkill::DeactiveSkillEffect, 10.f, false);
	ExecuteSkillEffect();

}

void UGS_AresUltimateSkill::DeactiveSkill()
{
	if (AGS_Seeker* OwnerPlayer = Cast<AGS_Seeker>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_SetIsFullBodySlot(false);
		OwnerPlayer->SetSkillInputControl(true, true, true);
		OwnerPlayer->SetMoveControlValue(true, true);
	}
}

void UGS_AresUltimateSkill::DeactiveSkillEffect()
{
	bIsBerserker = false;

	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		StatComp->SetInvincible(false);
		StatComp->ResetStat(BuffAmount);
	}

	

	// 쿨타임 복원
	if (UGS_SkillComp* SkillComp = OwnerCharacter->GetSkillComp())
	{
		if (UGS_SkillBase* MovingSkill = SkillComp->GetSkillFromSkillMap(ESkillSlot::Moving))
		{
			SkillComp->ResetCooldownModifier(ESkillSlot::Moving);
		}
		OriginalMovingSkillCooltime = -1.f; // 초기화
	}
}

void UGS_AresUltimateSkill::ExecuteSkillEffect()
{
	if (!OwnerCharacter)
	{
		return;
	}

	// 1. 데미지 무효화
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		StatComp->SetInvincible(true);
	}

	// 2~3. 스탯 변경
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		FGS_StatRow BuffStat;
		BuffStat.ATK = 50.f;     // 공격력 +50
		BuffStat.ATS = 0.5f;     // 공격속도 +0.5 (예시)

		BuffAmount = BuffStat; // 나중에 되돌릴 때 사용할 변수
		StatComp->ChangeStat(BuffStat);
	}

	// 4. AresMovingSkill 쿨타임 감소
	if (UGS_SkillComp* SkillComp = OwnerCharacter->GetSkillComp())
	{
		if (UGS_SkillBase* MovingSkill = SkillComp->GetSkillFromSkillMap(ESkillSlot::Moving))
		{
			SkillComp->ApplyCooldownModifier(ESkillSlot::Moving, 0.5);
		}
	}
}

bool UGS_AresUltimateSkill::IsActive() const
{
	return bIsBerserker;
}
