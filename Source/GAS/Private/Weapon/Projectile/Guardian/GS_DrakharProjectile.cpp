#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Character/GS_Character.h"

#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AGS_DrakharProjectile::AGS_DrakharProjectile()
{
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
}

void AGS_DrakharProjectile::BeginPlay()
{
	Super::BeginPlay();

}

void AGS_DrakharProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AGS_Character* DamagedCharacter = Cast<AGS_Character>(OtherActor);
	
	if (IsValid(DamagedCharacter))
	{
		FDamageEvent DamageEvent;
		DamagedCharacter->TakeDamage(120.f, DamageEvent, GetOwner()->GetInstigatorController(), this);
	}
	Destroy();
}