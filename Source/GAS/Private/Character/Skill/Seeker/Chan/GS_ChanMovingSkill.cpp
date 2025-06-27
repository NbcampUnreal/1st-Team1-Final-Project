// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Chan/GS_ChanMovingSkill.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Character/Debuff/EDebuffType.h"
#include "Character/Player/GS_Player.h"
#include "AkAudioEvent.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_StatRow.h"

UGS_ChanMovingSkill::UGS_ChanMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_ChanMovingSkill::ActiveSkill()
{
	if (!CanActive()) return;
	Super::ActiveSkill();
	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		
			OwnerPlayer->Multicast_SetIsFullBodySlot(true);
			OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
			OwnerPlayer->SetMoveControlValue(false, false);
			OwnerPlayer->SetSkillInputControl(false, false, false);
			OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
			
			if (OwnerCharacter->GetSkillComp())
			{
				OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Moving, true);
			}
			// =======================
			// VFX 재생 - 컴포넌트 RPC 사용
			// =======================
			if (OwningComp)
			{
				FVector SkillLocation = OwnerCharacter->GetActorLocation();
				FRotator SkillRotation = OwnerCharacter->GetActorRotation();
			
				// 스킬 시전 VFX 재생
				OwningComp->Multicast_PlayCastVFX(CurrentSkillType, SkillLocation, SkillRotation);
			
				// 스킬 범위 표시 VFX 재생
				OwningComp->Multicast_PlayRangeVFX(CurrentSkillType, SkillLocation, 800.0f);
			}
			
			// 무빙 스킬 사운드 재생
			const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
			if (SkillInfo && SkillInfo->SkillStartSound)
			{
				OwnerPlayer->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
			}

			ExecuteSkillEffect();
	}
}

void UGS_ChanMovingSkill::DeactiveSkill()
{
	Super::DeactiveSkill();
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);

		OwnerPlayer->Multicast_SetIsFullBodySlot(false);
		OwnerPlayer->Multicast_SetIsUpperBodySlot(false);
		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->SetSkillInputControl(true, true, true);
		OwnerPlayer->CanChangeSeekerGait = true;

		if (OwnerCharacter->GetSkillComp())
		{
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Moving, false);
		}
		
		// =======================
		// 스킬 종료 VFX 재생
		// =======================
		if (OwningComp)
		{
			FVector SkillLocation = OwnerCharacter->GetActorLocation();
			FRotator SkillRotation = OwnerCharacter->GetActorRotation();
			
			// 스킬 종료 VFX 재생
			OwningComp->Multicast_PlayEndVFX(CurrentSkillType, SkillLocation, SkillRotation);
		}

}

void UGS_ChanMovingSkill::InterruptSkill()
{
	Super::InterruptSkill();
	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);

		if (OwnerCharacter->GetSkillComp())
		{
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Moving, false);
		}

		OwnerCharacter->GetWorldTimerManager().ClearTimer(DEFBuffHandle); // SJE To KCY

}

void UGS_ChanMovingSkill::ExecuteSkillEffect()
{
	TArray<FHitResult> HitResults;

	const FVector Center = OwnerCharacter->GetActorLocation(); // 중심은 캐릭터
	const float Radius = 800.0f;

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);
	OwnerPlayer->Multicast_DrawSkillRange(Center, Radius, FColor::Red, 2.0f);
	// 캐릭터를 중심으로 한 지점에 고정된 SphereOverlap
	if (GetWorld()->SweepMultiByChannel(HitResults, Center, Center, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		TSet<AActor*> HitActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			UPrimitiveComponent* HitComponent = Hit.GetComponent();

			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}

			// SoundTrigger 콜리전 프로파일을 가진 컴포넌트 제외
			if (HitComponent && HitComponent->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}

			HitActors.Add(HitActor);

			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor))
			{
				ApplyEffectToDungeonMonster(TargetMonster);
				// Impact VFX 재생 (오프셋은 추후 함수 시그니처 변경 시 적용)
				TargetMonster->Multicast_PlayImpactVFX(SkillImpactVFX, SkillVFXScale);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor))
			{
				ApplyEffectToGuardian(TargetGuardian);
				// Impact VFX 재생 (오프셋은 추후 함수 시그니처 변경 시 적용)
				TargetGuardian->Multicast_PlayImpactVFX(SkillImpactVFX, SkillVFXScale);
			}
		}
	}
}

void UGS_ChanMovingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
	// 방어력 증가
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		FGS_StatRow BuffStat;
		BuffStat.DEF = 200.0f;     // 방어력 +50

		BuffAmount = BuffStat; // 나중에 되돌릴 때 사용할 변수
		StatComp->ChangeStat(BuffStat);
	}

	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(DEFBuffHandle, this, &UGS_ChanMovingSkill::DeactiveDEFBuff, 20.0f, false);

	// 어그로 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Aggro, OwnerCharacter);
	}
}

void UGS_ChanMovingSkill::ApplyEffectToGuardian(AGS_Guardian* Target)
{
	// 뮤트 디버프
	if (UGS_DebuffComp* DebuffComp = Target->FindComponentByClass<UGS_DebuffComp>())
	{
		Target->GetDebuffComp()->ApplyDebuff(EDebuffType::Mute, OwnerCharacter);
	}
}

void UGS_ChanMovingSkill::DeactiveDEFBuff()
{
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		StatComp->bIsInvincible = false;
		StatComp->ResetStat(BuffAmount);
	}
}

