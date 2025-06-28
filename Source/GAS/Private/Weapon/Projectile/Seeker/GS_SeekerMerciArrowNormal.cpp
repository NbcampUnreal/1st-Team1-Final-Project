// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "Weapon/Projectile/Component/GS_ArrowFXComponent.h"
#include "Weapon/Projectile/GS_TargetType.h"

#include "Character/Component/GS_StatComp.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Skill/GS_IgnoreDefenceDamageType.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GameFramework/DamageType.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGS_SeekerMerciArrowNormal::AGS_SeekerMerciArrowNormal()
{

}

void AGS_SeekerMerciArrowNormal::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGS_SeekerMerciArrowNormal::ProcessDamageLogic(ETargetType TargetType, const FHitResult& SweepResult, AActor* HitActor)
{
	// 중복 데미지 방지
	if (DamagedActors.Contains(HitActor))
	{
		return;
	}

	// 데미지를 적용할 타겟인지 확인
	if (TargetType != ETargetType::Guardian && TargetType != ETargetType::DungeonMonster)
	{
		return; // 가디언과 던전몬스터가 아니면 데미지 처리 안함
	}

	float DamageToApply = 0.f;
	AGS_Character* DamagedCharacter = Cast<AGS_Character>(HitActor);
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());

	TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

	// 화살 타입별 데미지 계산
	switch (ArrowType)
	{
	case EArrowType::Normal:
		if (IsValid(DamagedCharacter) && IsValid(OwnerCharacter))
		{
			DamageToApply = DamagedCharacter->GetStatComp()->CalculateDamage(OwnerCharacter, DamagedCharacter);
		}

		// 가디언에게는 50% 데미지
		if (TargetType == ETargetType::Guardian)
		{
			DamageToApply *= 0.5f;
		}
		break;

	case EArrowType::Axe:
		DamageTypeClass = UGS_IgnoreDefenceDamageType::StaticClass();
		if (IsValid(DamagedCharacter) && IsValid(OwnerCharacter))
		{
			// 방어력 무시 데미지 (세 번째 파라미터 1.f, 네 번째 파라미터 0.f)
			DamageToApply = DamagedCharacter->GetStatComp()->CalculateDamage(OwnerCharacter, DamagedCharacter, 1.f, 0.f);
		}
		break;

	case EArrowType::Child:
		if (IsValid(DamagedCharacter) && IsValid(OwnerCharacter))
		{
			DamageToApply = DamagedCharacter->GetStatComp()->CalculateDamage(OwnerCharacter, DamagedCharacter);
		}
		break;
	}

	// 데미지 적용
	if (DamageToApply > 0.f)
	{
		UGameplayStatics::ApplyPointDamage(
			HitActor,
			DamageToApply,
			GetActorForwardVector(),
			SweepResult,
			GetInstigatorController(),
			this,
			DamageTypeClass
		);

		// 데미지 적용 후 대상 기록 (중복 방지)
		DamagedActors.Add(HitActor);

		UE_LOG(LogTemp, Warning, TEXT("Damage Applied: %.2f to %s (ArrowType: %d)"),
			DamageToApply, *HitActor->GetName(), (int32)ArrowType);
	}
}

bool AGS_SeekerMerciArrowNormal::HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult)
{
	// 화살 타입과 타겟 타입에 따른 특별한 처리
	switch (ArrowType)
	{
	case EArrowType::Normal:
		if (TargetType == ETargetType::Guardian)
		{
			// 가디언에게는 박히지 않고 바로 파괴
			// 이동 중지
			if (ProjectileMovementComponent)
			{
				ProjectileMovementComponent->StopMovementImmediately();
				ProjectileMovementComponent->Deactivate();
			}
			SetActorEnableCollision(false);

			Destroy();
			return false; // 이동 중지 (파괴되므로)
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			// 던전 몬스터에게는 박힘 (ProcessStickLogic에서 처리됨)
		}
		break;

	case EArrowType::Axe:
		// 도끼 화살은 모든 대상에게 박힘
		if (TargetType == ETargetType::Guardian || TargetType == ETargetType::DungeonMonster)
		{
			// 박힘 처리는 ProcessStickLogic에서 자동으로 처리됨
		}
		break;

	case EArrowType::Child:
		if (TargetType == ETargetType::Guardian)
		{
			// 가디언에게는 박힘
		}
		else if (TargetType == ETargetType::DungeonMonster)
		{
			// 던전 몬스터에게는 관통 - 아무것도 하지 않음 (박히지도 않고 파괴되지도 않음)
			return true; // 관통: 이동 계속
		}
		break;
	}

	// 부모 클래스의 기본 처리 호출
	return Super::HandleTargetTypeGeneric(TargetType, SweepResult);
}

void AGS_SeekerMerciArrowNormal::ChangeArrowType(EArrowType Type)
{
	ArrowType = Type;

	if (ArrowFXComponent)
	{
		ArrowFXComponent->StartArrowTrailVFX(ArrowType);
	}
}

