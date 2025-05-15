// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Skill/GS_IgnoreDefenceDamageType.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GameFramework/DamageType.h"
#include "Components/SphereComponent.h"

AGS_SeekerMerciArrowNormal::AGS_SeekerMerciArrowNormal()
{

}

void AGS_SeekerMerciArrowNormal::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_SeekerMerciArrowNormal::OnBeginOverlap);
}

void AGS_SeekerMerciArrowNormal::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	// 중복 데미지 들어가지 않도록 맞은 사람들 저장(관통)
	HitActors.Add(OtherActor);

	ETargetType TargetType;
	// 가디언인지 구분
	if (Cast<AGS_Monster>(OtherActor))
	{
		TargetType = ETargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		TargetType = ETargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		TargetType = ETargetType::Seeker;
	}
	else
	{
		TargetType = ETargetType::Etc;
	}
	const bool bIsGuardian = Cast<AGS_Guardian>(OtherActor) != nullptr;

	// 데미지
	float DamageToApply = BaseDamage;
	TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

	switch (ArrowType)
	{
	case EArrowType::Normal:
		if (TargetType == ETargetType::Guardian)
		{
			DamageToApply *= 0.5; // 화살 데미지 감소
			Destroy(); // 삭제
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			StickWithVisualOnly(Hit); // 화살 박힘
		}
		break;
	case EArrowType::Axe:
		if (TargetType == ETargetType::Guardian || TargetType == ETargetType::DungeonMonster)
		{
			DamageTypeClass = UGS_IgnoreDefenceDamageType::StaticClass(); // 방어력 무시 데미지 타입 설정
			StickWithVisualOnly(Hit); // 화살 박힘
		}
		break;
	case EArrowType::Child:
		if (TargetType == ETargetType::Guardian)
		{
			StickWithVisualOnly(Hit); // 화살 박힘
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			// 관통 = 아무것도 적용하지 X
		}
		break;
	default:
		break;
	}
	

	if (TargetType == ETargetType::Etc)
	{
		StickWithVisualOnly(Hit); // 화살 박힘
	}
	else if (TargetType == ETargetType::Seeker)
	{

	}
	else if (TargetType == ETargetType::DungeonMonster || TargetType == ETargetType::Guardian)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, DamageToApply, GetActorForwardVector(), Hit, GetInstigatorController(), this, DamageTypeClass);
	}
}

void AGS_SeekerMerciArrowNormal::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	ETargetType TargetType;
	// 가디언인지 구분
	if (Cast<AGS_Monster>(OtherActor))
	{
		TargetType = ETargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		TargetType = ETargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		TargetType = ETargetType::Seeker;
	}
	else
	{
		TargetType = ETargetType::Etc;
	}

	float DamageToApply = BaseDamage;
	TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

	switch (ArrowType)
	{
	case EArrowType::Normal:
		if (TargetType == ETargetType::Guardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("Overlap - Normal - Guardian"));
			DamageToApply *= 0.5f;
			// 화살이 박히지 않음 → 바로 Destroy
			Destroy();
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			UE_LOG(LogTemp, Warning, TEXT("Overlap - Normal - Dungeon Monster"));
			StickWithVisualOnly(SweepResult);
		}
		break;

	case EArrowType::Axe:
		UE_LOG(LogTemp, Warning, TEXT("Overlap - Axe - Guardian, DungeonMonster"));
		DamageTypeClass = UGS_IgnoreDefenceDamageType::StaticClass();
		StickWithVisualOnly(SweepResult);
		break;

	case EArrowType::Child:
		if (TargetType == ETargetType::Guardian)
		{
			UE_LOG(LogTemp, Warning, TEXT("Overlap - Child - Guardian"));
			StickWithVisualOnly(SweepResult);
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			// 일반 몬스터에게는 관통 → 시각 효과 없이 그냥 지나감
			UE_LOG(LogTemp, Warning, TEXT("Overlap - Child - Dungeon Monster"));
		}
		
		break;
	}

	if (TargetType == ETargetType::Etc)
	{
		StickWithVisualOnly(SweepResult); // 화살 박힘
	}
	else if (TargetType == ETargetType::Seeker)
	{

	}
	else if (TargetType == ETargetType::DungeonMonster || TargetType == ETargetType::Guardian)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, DamageToApply, GetActorForwardVector(), SweepResult, GetInstigatorController(), this, DamageTypeClass);
	}
}

