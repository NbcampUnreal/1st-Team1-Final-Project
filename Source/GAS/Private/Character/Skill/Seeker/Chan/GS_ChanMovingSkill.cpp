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
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Component/GS_StatRow.h"
#include "Sound/GS_SeekerAudioComponent.h"

UGS_ChanMovingSkill::UGS_ChanMovingSkill()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_ChanMovingSkill::ActiveSkill()
{
	Super::ActiveSkill();

	// 스킬 쿨타임 측정 시작
	StartCoolDown();

	if (AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter))
	{
		// 애니메이션 설정
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::FullBody);

		// 입력 제한 설정
		OwnerPlayer->SetMoveControlValue(false, false);

		// 스킬 애니메이션 재생
		OwnerPlayer->Multicast_PlaySkillMontage(SkillAnimMontages[0]);

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
			
		// 스킬 시작 사운드 재생
		if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
		{
			if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
			{
				AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, true);
			}
		}

		// 방어력 강화
		StrengthenDefense();

		// 어그로
		AggroToOwner();
	}
}

void UGS_ChanMovingSkill::OnSkillCanceledByDebuff()
{
	Super::OnSkillCanceledByDebuff();
}

void UGS_ChanMovingSkill::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	AGS_Chan* OwnerPlayer = Cast<AGS_Chan>(OwnerCharacter);

	if(OwnerPlayer)
	{
		OwnerPlayer->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
		OwnerPlayer->SetMoveControlValue(true, true);
		OwnerPlayer->CanChangeSeekerGait = true;
	}

	SetIsActive(false);

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

	SetIsActive(false);

	OwnerPlayer->GetWorldTimerManager().ClearTimer(DEFBuffHandle);
}

void UGS_ChanMovingSkill::ApplyEffectToDungeonMonster(AGS_Monster* Target)
{
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

void UGS_ChanMovingSkill::DeactiveSkill()
{
	// 방어력 증가 디버프 해제
	DeactiveDEFBuff();

	// SeekerAudioComponent를 통한 스킬 종료 사운드
	if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerSeeker->SeekerAudioComponent)
		{
			AudioComp->PlaySkillSoundFromDataTable(CurrentSkillType, false);
		}
	}

	// 스킬 상태 업데이트
	Super::DeactiveSkill();
}

void UGS_ChanMovingSkill::AggroToOwner()
{
	// 범위 내 몬스터 인식 및 효과 활성화
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

			if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(HitActor)) // 몬스터일 경우
			{
				ApplyEffectToDungeonMonster(TargetMonster);
				// Impact VFX 재생 (오프셋은 추후 함수 시그니처 변경 시 적용)
				TargetMonster->Multicast_PlayImpactVFX(SkillImpactVFX, SkillVFXScale);
			}
			else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(HitActor)) // 가디언일 경우
			{
				ApplyEffectToGuardian(TargetGuardian);
				// Impact VFX 재생 (오프셋은 추후 함수 시그니처 변경 시 적용)
				TargetGuardian->Multicast_PlayImpactVFX(SkillImpactVFX, SkillVFXScale);
			}
		}
	}
}

void UGS_ChanMovingSkill::StrengthenDefense()
{
	// 방어력 증가
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		FGS_StatRow BuffStat;
		BuffStat.DEF = ExtraDefense;     // 방어력 *2(+200.0f)

		BuffAmount = BuffStat; // 나중에 되돌릴 때 사용할 변수
		StatComp->ChangeStat(BuffStat);
	}

	OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(DEFBuffHandle, this, &UGS_ChanMovingSkill::DeactiveSkill, StrengthenDefenseDuration, false);
}

void UGS_ChanMovingSkill::DeactiveDEFBuff()
{
	// 방어력 강화 버프 리셋
	if (UGS_StatComp* StatComp = OwnerCharacter->GetStatComp())
	{
		StatComp->ResetStat(BuffAmount);
	}
}

