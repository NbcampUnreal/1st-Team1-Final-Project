// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Guardian/GS_NeedleFangProjectile.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/GS_Character.h"
#include "Engine/DamageEvents.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "Character/F_GS_DamageEvent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/SphereComponent.h"
#include "Weapon/Equipable/GS_WeaponShield.h"
#include "VFX/GS_VFX_FunctionLibrary.h"

AGS_NeedleFangProjectile::AGS_NeedleFangProjectile()
{
	ProjectileLifeTime = 1.5f;
}

void AGS_NeedleFangProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AGS_NeedleFangProjectile::HandleProjectileDestroy, ProjectileLifeTime, false);
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_NeedleFangProjectile::OnBeginOverlap);

		// Overlap 설정 강화
		CollisionComponent->SetGenerateOverlapEvents(true);
	}
}

void AGS_NeedleFangProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp && OtherComp->GetCollisionProfileName() == FName("SoundTrigger"))
	{
		return;
	}

	AGS_Character* DamagedCharacter = Cast<AGS_Character>(OtherActor);
	// 맞은게 AGS_Character 타입이 아니라면
	if (!DamagedCharacter)
	{
		// Chan의 방패인지 확인
		if (AGS_WeaponShield* ChanShield = Cast<AGS_WeaponShield>(OtherActor))
		{
			DamagedCharacter = ChanShield->GetOwnerChar();
		}
	}
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());

	if (DamagedCharacter && OwnerCharacter && DamagedCharacter->IsEnemy(OwnerCharacter) && DamagedCharacter->GetStatComp())
	{
		Multicast_PlayHitSound(SweepResult.ImpactPoint);

		UGS_StatComp* DamagedStat = DamagedCharacter->GetStatComp();
		float Damage = DamagedStat->CalculateDamage(OwnerCharacter, DamagedCharacter);
		FGS_DamageEvent DamageEvent;
		DamageEvent.HitReactType = EHitReactType::DamageOnly;

		float ActualDamage = DamagedCharacter->TakeDamage(Damage, DamageEvent, GetOwner()->GetInstigatorController(), this);

		// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생
		if (ActualDamage > 0.0f)
		{
			Multicast_PlayBloodEffect(SweepResult.ImpactPoint, SweepResult.ImpactNormal);
		}
	}
	Destroy();
}

void AGS_NeedleFangProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                     FVector NormalImpulse, const FHitResult& Hit)
{
	
}

void AGS_NeedleFangProjectile::HandleProjectileDestroy()
{
	Destroy();
}

void AGS_NeedleFangProjectile::Multicast_PlayHitSound_Implementation(FVector HitLocation)
{
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}

	if (HitSoundEvent)
	{
		UAkGameplayStatics::PostEventAtLocation(
			HitSoundEvent,
			HitLocation,
			FRotator::ZeroRotator,
			GetWorld()
		);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("NeedleFang HitSoundEvent is null"));
	}
}

void AGS_NeedleFangProjectile::Multicast_PlayBloodEffect_Implementation(FVector HitLocation, FVector HitNormal)
{
	UGS_VFX_FunctionLibrary::PlayBloodEffect(this, BloodEffectSystem, HitLocation, FRotationMatrix::MakeFromZ(HitNormal).Rotator());
}