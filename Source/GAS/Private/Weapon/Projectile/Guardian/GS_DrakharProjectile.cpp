#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Character/GS_Character.h"

#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AGS_DrakharProjectile::AGS_DrakharProjectile()
{
	//CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn);
	//CollisionComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	//CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	//CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);

	//CollisionComponent->SetCollisio
	//CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AGS_DrakharProjectile::BeginPlay()
{
	Super::BeginPlay();

}

void AGS_DrakharProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// SoundTrigger 콜리전 프로파일을 가진 컴포넌트 제외
	if (OtherComp && OtherComp->GetCollisionProfileName() == FName("SoundTrigger"))
	{
		return;
	}

	AGS_Character* DamagedCharacter = Cast<AGS_Character>(OtherActor);
	
	if (IsValid(DamagedCharacter))
	{
		FDamageEvent DamageEvent;
		DamagedCharacter->TakeDamage(120.f, DamageEvent, GetOwner()->GetInstigatorController(), this);
	}
	Destroy();
}