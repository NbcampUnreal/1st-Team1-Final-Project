// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Guardian/GS_NeedleFangProjectile.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/DamageEvents.h"

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
	if (!DamagedCharacter || !OwnerCharacter || !DamagedCharacter->IsEnemy(OwnerCharacter))
	{
		return;
	}
	
	UGS_StatComp* DamagedStat = DamagedCharacter->GetStatComp();
	if (!DamagedStat) 
	{
		return;	
	}
	float Damage = DamagedStat->CalculateDamage(OwnerCharacter, DamagedCharacter);
	FDamageEvent DamageEvent;
	DamagedCharacter->TakeDamage(Damage, DamageEvent, GetOwner()->GetInstigatorController(), this);
	
	Destroy();
}

void AGS_NeedleFangProjectile::HandleProjectileDestroy()
{
	Destroy();
}