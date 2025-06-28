// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponSword.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

AGS_WeaponSword::AGS_WeaponSword()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	RootComponent = Mesh;

	HitBox = CreateDefaultSubobject<UBoxComponent>("HitBox");
	HitBox->SetupAttachment(Mesh);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponSword::OnHit);

	OwnerChar = nullptr;
	bReplicates = true;
}


void AGS_WeaponSword::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<AGS_Character>(GetOwner());
}

void AGS_WeaponSword::EnableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 히트 액터 목록 초기화 (새로운 공격 시작 시)
	HitActors.Empty();
}

void AGS_WeaponSword::DisableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_WeaponSword::ServerEnableHit_Implementation()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 히트 액터 목록 초기화 (새로운 공격 시작 시)
	HitActors.Empty();
}

void AGS_WeaponSword::ServerDisableHit_Implementation()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_WeaponSword::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	// 중복 히트 방지
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	// 맞은 대상 구분
	ESwordHitTargetType TargetType = DetermineTargetType(OtherActor);

	// HitResult 생성 (Overlap에서는 정확한 히트 포인트가 없을 수 있음)
	FHitResult CorrectHitResult = SweepResult;
	if (!bFromSweep)
	{
		CorrectHitResult.ImpactPoint = GetActorLocation();
		CorrectHitResult.Location = GetActorLocation();
		CorrectHitResult.ImpactNormal = FVector::UpVector;
		CorrectHitResult.Normal = FVector::UpVector;
	}

	Multicast_PlayHitSound(TargetType, CorrectHitResult);
	
	AGS_Character* Damaged = Cast<AGS_Character>(OtherActor);
	AGS_Character* Attacker = OwnerChar;

	if (!Damaged || !Attacker || !Damaged->IsEnemy(Attacker))
	{
		// 적이 아닌 대상(벽 등)을 타격한 경우, 기본 VFX만 재생하고 종료
		Multicast_PlayHitVFX(TargetType, CorrectHitResult);
		return;
	}

	// --- 여기서부터는 유효한 적을 타격한 경우 ---
	
	// 1. 기본 VFX는 항상 재생
	Multicast_PlayHitVFX(TargetType, CorrectHitResult);

	// 2. '아레스'의 특수 공격일 경우 추가 효과(사운드, VFX) 재생
	if (AGS_Ares* Ares = Cast<AGS_Ares>(Attacker))
	{
		if (Ares->CurrentComboIndex == 4)
		{
			// 추가 사운드
			Ares->Multicast_OnAttackHit(Ares->CurrentComboIndex);
			
			// 추가 VFX
			if (Ares->FinalAttackHitVFX)
			{
				Multicast_PlaySpecialHitVFX(Ares->FinalAttackHitVFX, CorrectHitResult);
			}
		}
	}
	
	UGS_StatComp* DamagedStat = Damaged->GetStatComp();
	if (!DamagedStat) 
	{
		return;	
	}

	float Damage = DamagedStat->CalculateDamage(Attacker, Damaged);
	FVector ShotDir = (Damaged->GetActorLocation() - OwnerChar->GetActorLocation()).GetSafeNormal();
	FPointDamageEvent DamageEvent;
	DamageEvent.ShotDirection = ShotDir;
	DamageEvent.HitInfo = SweepResult;
	DamageEvent.DamageTypeClass = UDamageType::StaticClass();
	Damaged->TakeDamage(Damage, DamageEvent, OwnerChar->GetController(), OwnerChar);

	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

ESwordHitTargetType AGS_WeaponSword::DetermineTargetType(AActor* OtherActor) const
{
	if (Cast<AGS_Monster>(OtherActor))
	{
		return ESwordHitTargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		return ESwordHitTargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		return ESwordHitTargetType::Seeker;
	}
	else if (Cast<AGS_Character>(OtherActor))
	{
		return ESwordHitTargetType::Other;
	}
	else
	{
		return ESwordHitTargetType::Structure;
	}
}

void AGS_WeaponSword::PlayHitSound(ESwordHitTargetType TargetType, const FHitResult& SweepResult)
{
	UAkAudioEvent* SoundEventToPlay = nullptr;

	switch (TargetType)
	{
	case ESwordHitTargetType::Guardian:
	case ESwordHitTargetType::DungeonMonster:
		SoundEventToPlay = HitPawnSoundEvent;
		break;
	case ESwordHitTargetType::Structure:
		SoundEventToPlay = HitStructureSoundEvent;
		break;
	case ESwordHitTargetType::Seeker:
	case ESwordHitTargetType::Other:
		break;
	default:
		break;
	}

	if (SoundEventToPlay && GetWorld())
	{
		UAkGameplayStatics::PostEventAtLocation(
			SoundEventToPlay,
			SweepResult.ImpactPoint,
			FRotator::ZeroRotator,
			GetWorld()
		);
	}
}

void AGS_WeaponSword::PlayHitVFX(ESwordHitTargetType TargetType, const FHitResult& SweepResult)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (TargetType)
	{
	case ESwordHitTargetType::Guardian:
	case ESwordHitTargetType::DungeonMonster:
		VFXToPlay = HitPawnVFX;
		break;
	case ESwordHitTargetType::Structure:
		VFXToPlay = HitStructureVFX;
		break;
	case ESwordHitTargetType::Seeker:
	case ESwordHitTargetType::Other:
		break;
	default:
		break;
	}

	if (VFXToPlay && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			SweepResult.ImpactPoint,
			SweepResult.ImpactNormal.Rotation(),
			FVector(1.0f),
			true,
			true
		);
	}
}

// 멀티캐스트 함수 구현
bool AGS_WeaponSword::Multicast_PlayHitSound_Validate(ESwordHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponSword::Multicast_PlayHitSound_Implementation(ESwordHitTargetType TargetType, const FHitResult& SweepResult)
{
	PlayHitSound(TargetType, SweepResult);
}

bool AGS_WeaponSword::Multicast_PlayHitVFX_Validate(ESwordHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponSword::Multicast_PlayHitVFX_Implementation(ESwordHitTargetType TargetType, const FHitResult& SweepResult)
{
	PlayHitVFX(TargetType, SweepResult);
}

void AGS_WeaponSword::Multicast_PlaySpecialHitVFX_Implementation(UNiagaraSystem* VFXToPlay, const FHitResult& HitResult)
{
	if (VFXToPlay && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			HitResult.ImpactPoint,
			HitResult.ImpactNormal.Rotation(),
			FVector(1.0f),
			true,
			true
		);
	}
}
