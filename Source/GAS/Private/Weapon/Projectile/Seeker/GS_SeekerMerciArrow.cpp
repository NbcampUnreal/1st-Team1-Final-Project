// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Weapon/Projectile/Component/GS_ArrowFXComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

AGS_SeekerMerciArrow::AGS_SeekerMerciArrow()
{
	// 화살 FX 컴포넌트 생성 (VFX + Sound)
	ArrowFXComponent = CreateDefaultSubobject<UGS_ArrowFXComponent>(TEXT("ArrowFXComponent"));
	
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

void AGS_SeekerMerciArrow::Multicast_InitHomingTarget_Implementation(AActor* Target)
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

		ProjectileMovementComponent->bRotationFollowsVelocity = true;
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
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->HomingTargetComponent = nullptr;
		ProjectileMovementComponent->bIsHomingProjectile = false;
		ProjectileMovementComponent->InitialSpeed = 5000.f;
		ProjectileMovementComponent->MaxSpeed = 5000.0f;
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

	//Crosshair Hit Anim
	if (TargetType == ETargetType::Guardian || TargetType == ETargetType::DungeonMonster)
	{
		if (AGS_Merci* MerciPlayer = Cast<AGS_Merci>(GetOwner()))
		{
			MerciPlayer->Client_ShowCrosshairHitFeedback();
		}
	}

	// 히트 사운드 & VFX 재생 (컴포넌트로 위임)
	if (ArrowFXComponent)
	{
		ArrowFXComponent->PlayHitSound(TargetType, SweepResult);
		ArrowFXComponent->PlayHitVFX(TargetType, SweepResult);
	}

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