// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Ares/GS_AresUltimateSkill.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_StatRow.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Player/Seeker/GS_Seeker.h"

UGS_AresUltimateSkill::UGS_AresUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}


void UGS_AresUltimateSkill::ActiveSkill()
{
	Super::ActiveSkill();

	// 쿨타임 측정 시작
	StartCoolDown();
	
	const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
	if (AGS_Ares* OwnerPlayer = Cast<AGS_Ares>(OwnerCharacter))
	{
		// 스킬 시작 사운드 재생
		if (UGS_SeekerAudioComponent* AudioComp = OwnerPlayer->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
		}
		
		// 궁극기 루프 사운드 재생 (SeekerAudioComponent만 지원)
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
		{
			if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
			{
				AudioComp->PlaySkillLoopSoundFromDataTable(CurrentSkillType);
			}
		}

		// 입력 제한 설정
		//OwnerPlayer->Multicast_SetIsFullBodySlot(true);
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		//OwnerPlayer->SetSkillInputControl(false, false, false, false);
		OwnerPlayer->SetMoveControlValue(false, false);
	}
	
	// =======================
	// VFX 재생 - 컴포넌트 RPC 사용
	// =======================
	if (OwningComp)
	{
		FVector SkillLocation = OwnerCharacter->GetActorLocation();
		//FRotator SkillRotation = OwnerCharacter->GetActorRotation();
		FRotator SkillRotation = FRotator(0.f, 0.f, 0.f);

		// 스킬 시전 VFX 재생
		OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
	}

	// DeactiveSkill 호출을 위한 타이머 설정
	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(UltimateSkillTimerHandle, this, &UGS_AresUltimateSkill::DeactiveSkill, 10.f, false);

	// 광전사화
	BecomeBerserker();
}

void UGS_AresUltimateSkill::OnSkillCanceledByDebuff()
{
}

void UGS_AresUltimateSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	if (AGS_Seeker* OwnerPlayer = Cast<AGS_Seeker>(OwnerCharacter))
	{
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		OwnerPlayer->SetMoveControlValue(true, true);
	}
}

void UGS_AresUltimateSkill::BecomeBerserker()
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

void UGS_AresUltimateSkill::DeactiveSkill()
{
	// 궁극기 루프 사운드 정지
	if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
		{
			AudioComp->StopSkillLoopSoundFromDataTable(CurrentSkillType);
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
		}
	}

	// 스탯 복원
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		StatComp->SetInvincible(false);
		StatComp->ResetStat(BuffAmount);
	}

	// 쿨타임 복원
	if (UGS_SkillComp* SkillComp = OwnerCharacter->GetSkillComp())
	{
		/*if (UGS_SkillBase* MovingSkill = SkillComp->GetSkillFromSkillMap(ESkillSlot::Moving))
		{
			SkillComp->ResetCooldownModifier(ESkillSlot::Moving);
		}*/
		SkillComp->ResetCooldownModifier(ESkillSlot::Moving);
		OriginalMovingSkillCooltime = -1.f; // 초기화
	}
	
	Super::DeactiveSkill();
}


