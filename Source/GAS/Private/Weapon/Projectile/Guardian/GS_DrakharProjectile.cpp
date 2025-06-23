#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AGS_DrakharProjectile::AGS_DrakharProjectile()
{
}

void AGS_DrakharProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (GetInstigator() && CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
		}
	}
}

void AGS_DrakharProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// SoundTrigger 콜리전 프로파일을 가진 컴포넌트 제외
	if (OtherComp && OtherComp->GetCollisionProfileName() == FName("SoundTrigger"))
	{
		return;
	}

	// === 충돌 이펙트 처리 ===
	bool bHitCharacter = false;
	AGS_Character* DamagedCharacter = Cast<AGS_Character>(OtherActor);
	
	if (IsValid(DamagedCharacter))
	{
		bHitCharacter = true;
		FDamageEvent DamageEvent;
		//UE_LOG(LogTemp, Warning, TEXT("DAMAGED!!!!"));
		DamagedCharacter->TakeDamage(120.f, DamageEvent, GetOwner()->GetInstigatorController(), this);
	}

	// === 소유자에게 충돌 이벤트 알림 ===
	if (AGS_Drakhar* OwnerDrakhar = Cast<AGS_Drakhar>(GetOwner()))
	{
		FVector ImpactLocation = Hit.ImpactPoint;
		FVector ImpactNormal = Hit.ImpactNormal;
		
		// 충돌 정보가 유효하지 않은 경우 투사체 위치 사용
		if (ImpactLocation.IsZero())
		{
			ImpactLocation = GetActorLocation();
		}
		if (ImpactNormal.IsZero())
		{
			ImpactNormal = -GetActorForwardVector(); // 투사체 진행 방향의 반대
		}
		
		// Drakhar에게 충돌 이펙트 재생 요청
		OwnerDrakhar->HandleDraconicProjectileImpact(ImpactLocation, ImpactNormal, bHitCharacter);
	}

	Destroy();
}