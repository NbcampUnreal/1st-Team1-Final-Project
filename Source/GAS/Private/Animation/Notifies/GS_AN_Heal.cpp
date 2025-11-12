// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_AN_Heal.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Skill/Seeker/GS_HealSkill.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Sound/GS_SeekerAudioComponent.h"

void UGS_AN_Heal::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner());

	if (!Seeker)
	{
		return;
	}
	
	UGS_StatComp* StatComp = Seeker->GetStatComp();

	if (!StatComp)
	{
		return;
	}

	UGS_HealSkill* HealSkill = Cast<UGS_HealSkill>(Seeker->GetSkillComp()->GetSkillFromSkillMap(ESkillSlot::HealPotion));

	if (!HealSkill)
	{
		return;
	}

	if (Seeker->HasAuthority() && !Seeker->IsDead())
	{
		float NewHealth = FMath::Min(StatComp->GetCurrentHealth() + HealSkill->GetHealAmount(), StatComp->GetMaxHealth());
		StatComp->SetCurrentHealth(NewHealth, true);

		// 포션 개수 감소
		HealSkill->DecreaseCurrentHealCount();

		// UI 업데이트를 위해 클라이언트에게 알림.

		if (UGS_SkillComp* SkillComp = Cast<UGS_SkillComp>(Seeker->GetSkillComp()))
		{
			// Cast VFX: 스킬 시전 시 플레이어 위치에 표시
			if (HealSkill->SkillCastVFX)
			{
				SkillComp->Multicast_PlayCastVFX(HealSkill->CurrentSkillType, Seeker->GetActorLocation(), Seeker->GetActorRotation());
			}

			// Impact VFX: 힐링 효과를 플레이어에게 표시
			if (HealSkill->SkillImpactVFX)
			{
				SkillComp->Multicast_PlayImpactVFX(HealSkill->CurrentSkillType, Seeker->GetActorLocation());
			}

			SkillComp->Client_BroadcastHealCountChanged(HealSkill->CurrentSkillType, HealSkill->GetCurrentHealCount(), HealSkill->GetMaxHealCount());
		}

		// SFX 재생 (모든 클라이언트에 동기화)
		if (UGS_SeekerAudioComponent* AudioComp = Seeker->FindComponentByClass<UGS_SeekerAudioComponent>())
		{
			// Multicast RPC 직접 호출 (CanSendRPC 체크 우회)
			// AudioEventType 0 = 스킬 시작 사운드
			AudioComp->Multicast_RequestSkillAudio(HealSkill->CurrentSkillType, 0, Seeker->GetActorLocation());
		}
	}
}
