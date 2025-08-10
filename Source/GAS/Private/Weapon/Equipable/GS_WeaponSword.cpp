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
#include "Character/F_GS_DamageEvent.h"

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
		// 히트 포인트 계산
		CorrectHitResult = CalculateMoreAccurateHitPoint(OtherActor);
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
	/*FPointDamageEvent DamageEvent;
	DamageEvent.ShotDirection = ShotDir;
	DamageEvent.HitInfo = SweepResult;
	DamageEvent.DamageTypeClass = UDamageType::StaticClass();*/
	FGS_DamageEvent DamageEvent;
	DamageEvent.HitReactType =  EHitReactType::Interrupt;
	Damaged->TakeDamage(Damage, DamageEvent, OwnerChar->GetController(), OwnerChar);

	// 한 번의 공격에 한 명의 적만 맞도록 히트박스 비활성화
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
	case ESwordHitTargetType::Seeker:
		SoundEventToPlay = HitSeekerSoundEvent;
		break;
	case ESwordHitTargetType::Structure:
		SoundEventToPlay = HitStructureSoundEvent;
		break;
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
	case ESwordHitTargetType::Seeker:
		VFXToPlay = HitSeekerVFX;
		break;
	case ESwordHitTargetType::Structure:
		VFXToPlay = HitStructureVFX;
		break;
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

FHitResult AGS_WeaponSword::CalculateMoreAccurateHitPoint(AActor* OtherActor) const
{
	FHitResult ResultHit;
	
	if (!OtherActor || !HitBox || !GetWorld())
	{
		// 기본값으로 무기 위치 사용
		ResultHit.ImpactPoint = GetActorLocation();
		ResultHit.Location = GetActorLocation();
		ResultHit.ImpactNormal = FVector::UpVector;
		ResultHit.Normal = FVector::UpVector;
		return ResultHit;
	}

	// HitBox의 월드 위치와 타겟의 위치 계산
	FVector HitBoxLocation = HitBox->GetComponentLocation();
	FVector TargetLocation = OtherActor->GetActorLocation();
	
	// HitBox에서 타겟으로의 방향 벡터
	FVector TraceDirection = (TargetLocation - HitBoxLocation).GetSafeNormal();
	
	// Line Trace 거리 (HitBox 크기의 2배 정도)
	float TraceDistance = FVector::Dist(HitBoxLocation, TargetLocation) + 100.0f;
	
	// Line Trace 시작점과 끝점
	FVector TraceStart = HitBoxLocation;
	FVector TraceEnd = HitBoxLocation + (TraceDirection * TraceDistance);
	
	// Line Trace 파라미터 설정
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 무기 자체는 무시
	QueryParams.AddIgnoredActor(GetOwner()); // 무기 소유자도 무시
	QueryParams.bTraceComplex = false;
	
	// Line Trace 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		ResultHit,
		TraceStart,
		TraceEnd,
		ECC_Pawn, // Pawn 채널로 트레이스
		QueryParams
	);
	
	if (bHit && ResultHit.GetActor() == OtherActor)
	{
		// 트레이스가 성공하고 올바른 타겟을 맞췄다면 해당 결과 사용
	}
	else
	{
		// 트레이스가 실패했다면 두 객체 간의 중점 계산
		FVector MidPoint = (HitBoxLocation + TargetLocation) * 0.5f;
		FVector ToTarget = (TargetLocation - HitBoxLocation).GetSafeNormal();
		
		ResultHit.ImpactPoint = MidPoint;
		ResultHit.Location = MidPoint;
		ResultHit.ImpactNormal = -ToTarget; // 타겟을 향하는 반대 방향
		ResultHit.Normal = -ToTarget;
	}
	
	return ResultHit;
}