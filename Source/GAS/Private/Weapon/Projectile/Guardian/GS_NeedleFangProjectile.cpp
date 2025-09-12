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
		// 히트 사운드 재생
		Multicast_PlayHitSound(Hit.ImpactPoint);
        
		UGS_StatComp* DamagedStat = DamagedCharacter->GetStatComp();
		float Damage = DamagedStat->CalculateDamage(OwnerCharacter, DamagedCharacter);
		FGS_DamageEvent DamageEvent;
		DamageEvent.HitReactType = EHitReactType::Interrupt;
		DamagedCharacter->TakeDamage(Damage, DamageEvent, GetOwner()->GetInstigatorController(), this);
	}
	
	Destroy();
}

void AGS_NeedleFangProjectile::HandleProjectileDestroy()
{
	Destroy();
}

void AGS_NeedleFangProjectile::Multicast_PlayHitSound_Implementation(FVector HitLocation)
{
	// 데디케이티드 서버에서는 사운드 재생하지 않음
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