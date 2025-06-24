// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_SwordAuraProjectile.generated.h"

class UBoxComponent;
class USphereComponent;
class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class GAS_API AGS_SwordAuraProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_SwordAuraProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UBoxComponent* SlashBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* SlashVFX;


	UPROPERTY()
	TSet<AActor*> HitActors;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 10.0f;

	// Overlap 함수
	UFUNCTION()
	void OnSlashBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	FTimerHandle DestorySwordAuraHandle;
	float SwordAuraLifetime = 1.0f;

	UFUNCTION()
	void DestroySwordAura();


	//VFX
	UFUNCTION(BlueprintCallable, Category = "Sword FX")
	void StartSwordSlashVFX();

	UFUNCTION(BlueprintCallable, Category = "Sword FX")
	void StopSwordSlashVFX();



	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartSwordSlashVFX();
	void Multicast_StartSwordSlashVFX_Implementation();


};
