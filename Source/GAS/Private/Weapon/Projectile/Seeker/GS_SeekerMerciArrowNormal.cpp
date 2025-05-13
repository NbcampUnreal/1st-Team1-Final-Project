// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowNormal.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Skill/GS_IgnoreDefenceDamageType.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GameFramework/DamageType.h"

AGS_SeekerMerciArrowNormal::AGS_SeekerMerciArrowNormal()
{

}

void AGS_SeekerMerciArrowNormal::BeginPlay()
{

}

void AGS_SeekerMerciArrowNormal::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	// 중복 데미지 들어가지 않도록 맞은 사람들 저장(관통)
	HitActors.Add(OtherActor);

	// 가디언인지 구분
	const bool bIsGuardian = Cast<AGS_Guardian>(OtherActor) != nullptr;

	// 데미지
	float DamageToApply = BaseDamage;
	TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

	switch (ArrowType)
	{
	case EArrowType::Normal:
		if (bIsGuardian)
		{
			DamageToApply *= 0.5; // 화살 데미지 감소
			Destroy(); // 삭제
		}
		else
		{
			StickWithVisualOnly(Hit); // 화살 박힘
		}
		break;
	case EArrowType::Axe:
		DamageTypeClass = UGS_IgnoreDefenceDamageType::StaticClass(); // 방어력 무시 데미지 타입 설정
		StickWithVisualOnly(Hit); // 화살 박힘
		break;
	case EArrowType::Child:
		if (bIsGuardian)
		{
			StickWithVisualOnly(Hit); // 화살 박힘
		}
		else
		{
			// 관통 = 아무것도 적용하지 X
		}
		break;
	default:
		break;
	}
	UGameplayStatics::ApplyPointDamage(OtherActor, DamageToApply, GetActorForwardVector(), Hit, GetInstigatorController(), this, DamageTypeClass);

	// 관통 화살이 아니거나 보스면 파괴
	if (ArrowType != EArrowType::Child || bIsGuardian)
	{
		Destroy();
	}
}
