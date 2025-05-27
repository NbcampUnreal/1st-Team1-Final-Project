// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Equipable/GS_WeaponMeleeBase.h"
#include "Character/GS_Character.h"
//#include "Character/Component/GS_StatComp.h"
//#include "Engine/DamageEvents.h"
//#include "Kismet/GameplayStatics.h"

AGS_WeaponMeleeBase::AGS_WeaponMeleeBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	// 히트박스 생성
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	HitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponMeleeBase::OnHit);
}

void AGS_WeaponMeleeBase::BeginPlay()
{
	Super::BeginPlay();
	SetOwningCharacter(Cast<AGS_Character>(GetOwner()));
}

void AGS_WeaponMeleeBase::EnableHit()
{
	if (HasAuthority())
	{
		SetHitCollision(true);
	}
	else
	{
		Server_SetHitCollision(true);
	}
}

void AGS_WeaponMeleeBase::DisableHit()
{
	if (HasAuthority())
	{
		SetHitCollision(false);
	}
	else
	{
		Server_SetHitCollision(false);
	}
}

void AGS_WeaponMeleeBase::Server_SetHitCollision_Implementation(bool bEnable)
{
	SetHitCollision(bEnable);
}

void AGS_WeaponMeleeBase::SetHitCollision(bool bEnable)
{
	if (HitBox)
	{
		HitBox->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void AGS_WeaponMeleeBase::StartAttack()
{
	// 새로운 공격 시 이전 히트 기록 초기화
	EnableHit();
}

void AGS_WeaponMeleeBase::EndAttack()
{
	DisableHit();
}

bool AGS_WeaponMeleeBase::IsValidTarget(AActor* Target) const
{
	// 기본 검사 수행
	if (!Super::IsValidTarget(Target))
	{
		return false;
	}

	// 캐릭터만 히트 가능
	AGS_Character* TargetCharacter = Cast<AGS_Character>(Target);
	if (!TargetCharacter)
	{
		return false;
	}

	// 소유자와 같은 팀인지 확인 (팀 시스템)
	// if (OwnerCharacter && TargetCharacter->GetTeamID() == OwnerCharacter->GetTeamID())
	// {
	//     return false;
	// }

	return true;
}

void AGS_WeaponMeleeBase::ProcessHit(AActor* HitActor)
{
	AGS_Character* DamagedCharacter = Cast<AGS_Character>(HitActor);
	if (!DamagedCharacter || !OwnerCharacter)
	{
		return;
	}

	// 데미지 계산 및 적용
	// ... 데미지 로직 ..
	
	// 히트 사운드 재생
	PlayHitSound(HitSoundEvent); // 로컬에서만 재생
}

void AGS_WeaponMeleeBase::PlayHitSound(UAkAudioEvent* SoundEvent)
{
	if (SoundEvent)
	{
		UAkGameplayStatics::PostEvent(
			SoundEvent,                  // AkAudioEvent
			this,                        // Actor
			0,                           // CallbackMask (기본값)
			FOnAkPostEventCallback()    // PostEventCallback
		);
	}
}