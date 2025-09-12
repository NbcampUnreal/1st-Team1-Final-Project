// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponEquipable.h"
#include "Weapon/Component/GS_WeaponVFXComponent.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"

AGS_WeaponEquipable::AGS_WeaponEquipable()
{
	OwnerChar = nullptr;
	bReplicates = true;

	// 무기 VFX 컴포넌트 생성
	WeaponVFXComponent = CreateDefaultSubobject<UGS_WeaponVFXComponent>(TEXT("WeaponVFXComponent"));
}

void AGS_WeaponEquipable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Set Server option
	SetReplicateMovement(true); // Replicate Actor Rotation & Transition
}

// 헬퍼 함수 구현
bool AGS_WeaponEquipable::IsValidForLevelTransition() const
{
	return IsValid(this) && GetWorld() && !GetWorld()->bIsTearingDown;
}

bool AGS_WeaponEquipable::IsOwnerCharValid() const
{
	return OwnerChar != nullptr && IsValid(OwnerChar) && !OwnerChar->IsActorBeingDestroyed();
}

void AGS_WeaponEquipable::ClearHitActors()
{
	HitActors.Empty();
}

void AGS_WeaponEquipable::ClearSafetyTimer()
{
	// 안전한 타이머 정리 - 레벨 전환 시에도 안전하게 처리
	if (UWorld* World = GetWorld(); World && IsValid(World) && !World->bIsTearingDown)
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (&TimerManager && SafetyTimerHandle.IsValid())
		{
			TimerManager.ClearTimer(SafetyTimerHandle);
		}
	}
	
	// 타이머 핸들 무효화
	SafetyTimerHandle.Invalidate();
}

FHitResult AGS_WeaponEquipable::CreateCorrectHitResult(const FHitResult& OriginalResult, bool bFromSweep) const
{
	FHitResult CorrectHitResult = OriginalResult;
	if (!bFromSweep)
	{
		CorrectHitResult.ImpactPoint = GetActorLocation();
		CorrectHitResult.Location = GetActorLocation();
		CorrectHitResult.ImpactNormal = FVector::UpVector;
		CorrectHitResult.Normal = FVector::UpVector;
	}
	return CorrectHitResult;
}

void AGS_WeaponEquipable::TriggerHitAuraOnHit(AGS_Character* HitTarget)
{
	if (!ShouldTriggerAuraOnHit(HitTarget) || !WeaponVFXComponent)
	{
		return;
	}

	// 시커 캐릭터 타입 확인
	ESeekerAuraType AuraType = GetSeekerAuraType(OwnerChar);
	if (AuraType != ESeekerAuraType::Default)
	{
		// 타격 시 아우라 활성화
		WeaponVFXComponent->ActivateHitAura(AuraType);
	}
}

ESeekerAuraType AGS_WeaponEquipable::GetSeekerAuraType(AGS_Character* SeekerChar) const
{
	if (!SeekerChar)
	{
		return ESeekerAuraType::Default;
	}

	// 시커 캐릭터 타입에 따른 아우라 타입 결정
	if (Cast<AGS_Chan>(SeekerChar))
	{
		return ESeekerAuraType::Chan;
	}
	else if (Cast<AGS_Ares>(SeekerChar))
	{
		return ESeekerAuraType::Ares;
	}
	else if (Cast<AGS_Merci>(SeekerChar))
	{
		return ESeekerAuraType::Merci;
	}

	return ESeekerAuraType::Default;
}

bool AGS_WeaponEquipable::ShouldTriggerAuraOnHit(AGS_Character* HitTarget) const
{
	// 기본 조건: 가디언이나 몬스터를 타격했을 때만 아우라 활성화
	return Cast<AGS_Guardian>(HitTarget) || Cast<AGS_Monster>(HitTarget);
}
