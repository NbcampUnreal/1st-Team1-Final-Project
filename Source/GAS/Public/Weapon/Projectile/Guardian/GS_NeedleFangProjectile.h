// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_NeedleFangProjectile.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_NeedleFangProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_NeedleFangProjectile();

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileLifeTime;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit) override;

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle DestroyTimerHandle;

	UFUNCTION() 
	void HandleProjectileDestroy();
};
