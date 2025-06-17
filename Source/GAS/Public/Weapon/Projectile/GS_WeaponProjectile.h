// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GS_Weapon.h"
#include "GS_WeaponProjectile.generated.h"

class UProjectileMovementComponent;
/**
 * 
 */
UCLASS()
class GAS_API AGS_WeaponProjectile : public AGS_Weapon
{
	GENERATED_BODY()

public:
	AGS_WeaponProjectile();

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* ProjectileMesh;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	UProjectileMovementComponent* GetProjectileMovement() { return ProjectileMovementComponent; }
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComponent;
	
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
};
