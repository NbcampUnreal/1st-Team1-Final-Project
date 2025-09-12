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

AGS_NeedleFangProjectile::AGS_NeedleFangProjectile()
{
	ProjectileLifeTime = 1.5f;
}

void AGS_NeedleFangProjectile::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AGS_NeedleFangProjectile::HandleProjectileDestroy, ProjectileLifeTime, false);
}

void AGS_NeedleFangProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                     FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp && OtherComp->GetCollisionProfileName() == FName("SoundTrigger"))
	{
		return;
	}

	AGS_Character* DamagedCharacter = Cast<AGS_Character>(OtherActor);
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (DamagedCharacter && OwnerCharacter && DamagedCharacter->IsEnemy(OwnerCharacter) && DamagedCharacter->GetStatComp())
	{
		Multicast_PlayHitSound(Hit.ImpactPoint);
        
		UGS_StatComp* DamagedStat = DamagedCharacter->GetStatComp();
		float Damage = DamagedStat->CalculateDamage(OwnerCharacter, DamagedCharacter);
		FGS_DamageEvent DamageEvent;
		DamageEvent.HitReactType = EHitReactType::DamageOnly;
		
		float ActualDamage = DamagedCharacter->TakeDamage(Damage, DamageEvent, GetOwner()->GetInstigatorController(), this);
		
		// 실제로 데미지가 적용된 경우에만 혈흔 이펙트 재생
		if (ActualDamage > 0.0f)
		{
			Multicast_PlayBloodEffect(Hit.ImpactPoint, Hit.ImpactNormal);
		}
	}
	
	Destroy();
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
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}

	UNiagaraSystem* EffectToPlay = BloodEffectSystem;
	
	// BloodEffectSystem이 없으면 기본 혈흔 이펙트 사용
	if (!EffectToPlay)
	{
		EffectToPlay = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/RealisticBlood/Burst/Niagara/NS_BloodBurst_High.NS_BloodBurst_High"));
	}
	
	if (EffectToPlay && GetWorld())
	{
		// 혈흔 이펙트의 회전을 충돌 법선에 맞춰 설정
		FRotator EffectRotation = FRotationMatrix::MakeFromZ(HitNormal).Rotator();
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			EffectToPlay,
			HitLocation,
			EffectRotation,
			FVector(1.0f, 1.0f, 1.0f), // 기본 스케일
			true,
			true
		);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("NeedleFang BloodEffect could not be loaded or spawned"));
	}
}