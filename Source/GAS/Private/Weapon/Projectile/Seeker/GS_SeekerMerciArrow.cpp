// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Components/SphereComponent.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/Seeker/GS_FieldSkillActor.h"

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
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_SeekerMerciArrow::OnBeginOverlap);
		this->SetActorEnableCollision(true);

		AActor* IgnoredActor = GetInstigator(); // 또는 GetInstigator();
		if (IgnoredActor)
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

	AGS_ArrowVisualActor* VisualArrow = GetWorld()->SpawnActor<AGS_ArrowVisualActor>(VisualArrowClass, SpawnLocation, AdjustedRotation, Params);

	if (VisualArrow)
	{
		VisualArrow->SetArrowMesh(ProjectileMesh->GetSkeletalMeshAsset());
		VisualArrow->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
	}

	// 원래 화살 제거
	Destroy();
}

void AGS_SeekerMerciArrow::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) 
	{
		return;
	}

	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	UE_LOG(LogTemp, Error, TEXT("Arrow hit actor: %s"), *OtherActor->GetName());

	// 맞은 대상 구분
	ETargetType TargetType = DetermineTargetType(OtherActor);

	// 공통 처리
	HandleTargetTypeGeneric(TargetType, SweepResult);
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
