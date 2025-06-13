// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Components/SphereComponent.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

AGS_SeekerMerciArrow::AGS_SeekerMerciArrow()
{
	if (HasAuthority())
	{
		// 화살 스폰 직후
		this->SetActorEnableCollision(false);
	}
}

void AGS_SeekerMerciArrow::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		if (CollisionComponent)
		{
			CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_SeekerMerciArrow::OnBeginOverlap);
		}
		this->SetActorEnableCollision(true);

		AActor* IgnoredActor = GetInstigator();
		if (IgnoredActor && CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(IgnoredActor, true);
		}
	}	
}

void AGS_SeekerMerciArrow::StickWithVisualOnly(const FHitResult& Hit)
{
	if (!ProjectileMesh)
	{
		return;
	}

	// 화살이 박힌 위치
	FVector SpawnLocation = Hit.ImpactPoint;

	// 방향 = 화살이 실제 날아온 방향
	FVector ArrowDirection = GetVelocity().GetSafeNormal();
	FRotator SpawnRotation = FRotationMatrix::MakeFromX(ArrowDirection).Rotator();

	// 메시가 +Y 방향을 앞이라고 가정 시 Y축 기준 -90도 회전
	FRotator AdjustedRotation = SpawnRotation + FRotator(0.f, -90.f, 0.f);
	SpawnLocation -= ArrowDirection * 25.f; // 약간 파고들게

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (VisualArrowClass && GetWorld())
	{
		AGS_ArrowVisualActor* VisualArrow = GetWorld()->SpawnActor<AGS_ArrowVisualActor>(VisualArrowClass, SpawnLocation, AdjustedRotation, Params);

		if (VisualArrow)
		{
			VisualArrow->SetArrowMesh(ProjectileMesh->GetSkeletalMeshAsset());
			if (Hit.BoneName != NAME_None)
			{
				VisualArrow->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
			}
			else
			{
				VisualArrow->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}

	// 원래 화살 제거
	Destroy();
}

void AGS_SeekerMerciArrow::InitHomingTarget(AActor* Target)
{
	if (!ProjectileMovementComponent)
	{
		return;
	}

	if(Target)
	{
		UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Target->GetRootComponent());
		if (!RootPrim)
		{
			UE_LOG(LogTemp, Error, TEXT("HomingTargetComponent is not a primitive component!"));
			return;
		}

		ProjectileMovementComponent->HomingTargetComponent = RootPrim;
		ProjectileMovementComponent->bIsHomingProjectile = true;
		ProjectileMovementComponent->InitialSpeed = 3000.f;
		ProjectileMovementComponent->MaxSpeed = 3000.f;
		ProjectileMovementComponent->HomingAccelerationMagnitude = 20000.f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
		ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;

		HomingTarget = Target;
		UE_LOG(LogTemp, Warning, TEXT("HomingTarget set to %s"), *Target->GetName());
	}
	else
	{
		// 유도 해제 : 일반 직선 화살로 설정
		ProjectileMovementComponent->HomingTargetComponent = nullptr;
		ProjectileMovementComponent->bIsHomingProjectile = false;
		ProjectileMovementComponent->InitialSpeed = 6000.0f;
		ProjectileMovementComponent->MaxSpeed = 6000.0f;
		ProjectileMovementComponent->ProjectileGravityScale = 1.0f;

		HomingTarget = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("HomingTarget is null — switching to normal arrow"));

	}

	// 방향성 초기화
	ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;
}

void AGS_SeekerMerciArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	UE_LOG(LogTemp, Error, TEXT("Arrow hit actor: %s"), *OtherActor->GetName());

	// 맞은 대상 구분
	ETargetType TargetType = DetermineTargetType(OtherActor);

	// 히트 사운드 재생 (멀티캐스트)
	Multicast_PlayHitSound(TargetType, SweepResult);

	// 히트 VFX 재생 (멀티캐스트)
	Multicast_PlayHitVFX(TargetType, SweepResult);

	// 서버에서만 로직 처리
	if (HasAuthority())
	{
		HandleTargetTypeGeneric(TargetType, SweepResult);
	}
}

ETargetType AGS_SeekerMerciArrow::DetermineTargetType(AActor* OtherActor) const
{
	if (Cast<AGS_Monster>(OtherActor))
	{
		return ETargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		return ETargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		return ETargetType::Seeker;
	}
	else if (Cast<AGS_SeekerMerciArrow>(OtherActor) || Cast<AGS_FieldSkillActor>(OtherActor))
	{
		return ETargetType::Skill;
	}
	else
	{
		return ETargetType::Structure;
	}
}

void AGS_SeekerMerciArrow::HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult)
{
	switch (TargetType)
	{
	case ETargetType::Skill:
		break;
	case ETargetType::Structure:
		StickWithVisualOnly(SweepResult);
		break;
	case ETargetType::Seeker:
		break;
	default:
		break;
	}
}

void AGS_SeekerMerciArrow::PlayHitSound(ETargetType TargetType, const FHitResult& SweepResult)
{
	UAkAudioEvent* SoundEventToPlay = nullptr;

	switch (TargetType)
	{
	case ETargetType::Guardian:
	case ETargetType::DungeonMonster:
		// 적 캐릭터(가디언, 던전몬스터)에 맞았을 때 Wwise 이벤트
		SoundEventToPlay = HitPawnSoundEvent;
		break;
	case ETargetType::Structure:
		// 벽이나 구조물에 맞았을 때 Wwise 이벤트
		SoundEventToPlay = HitStructureSoundEvent;
		break;
	case ETargetType::Seeker:
	case ETargetType::Skill:
		// 아군 시커나 스킬에 맞았을 때는 사운드 없음
		break;
	default:
		break;
	}

	// Wwise 사운드 재생
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

void AGS_SeekerMerciArrow::PlayHitVFX(ETargetType TargetType, const FHitResult& SweepResult)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (TargetType)
	{
	case ETargetType::Guardian:
	case ETargetType::DungeonMonster:
		// 적 캐릭터(가디언, 던전몬스터)에 맞았을 때 VFX
		VFXToPlay = HitPawnVFX;
		break;
	case ETargetType::Structure:
		// 벽이나 구조물에 맞았을 때 VFX
		VFXToPlay = HitStructureVFX;
		break;
	case ETargetType::Seeker:
	case ETargetType::Skill:
		// 아군 시커나 스킬에 맞았을 때는 VFX 없음
		break;
	default:
		break;
	}

	// VFX 재생
	if (VFXToPlay && GetWorld())
	{
		// 히트 포인트에서 VFX 재생
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			SweepResult.ImpactPoint,
			SweepResult.ImpactNormal.Rotation(), // 히트 표면의 법선 방향으로 VFX 회전
			FVector(1.0f), // Scale (float → FVector)
			true, // Auto Destroy
			true  // Auto Activate
		);
	}
}

// 멀티캐스트 함수 구현
bool AGS_SeekerMerciArrow::Multicast_PlayHitSound_Validate(ETargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_SeekerMerciArrow::Multicast_PlayHitSound_Implementation(ETargetType TargetType, const FHitResult& SweepResult)
{
	PlayHitSound(TargetType, SweepResult);
}

bool AGS_SeekerMerciArrow::Multicast_PlayHitVFX_Validate(ETargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_SeekerMerciArrow::Multicast_PlayHitVFX_Implementation(ETargetType TargetType, const FHitResult& SweepResult)
{
	PlayHitVFX(TargetType, SweepResult);
}
