#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_DrakharProjectile.generated.h"


UCLASS()
class GAS_API AGS_DrakharProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()
	
public:
	AGS_DrakharProjectile();

	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit) override;
};
