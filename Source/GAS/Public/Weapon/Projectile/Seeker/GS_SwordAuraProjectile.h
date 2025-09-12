// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_SwordAuraProjectile.generated.h"

class UBoxComponent;
class USphereComponent;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class ESwordAuraEffectType : uint8
{
	LeftNormal,
	RightNormal,
	LeftBuff,
	RightBuff
};

UCLASS()
class GAS_API AGS_SwordAuraProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_SwordAuraProjectile();

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	ESwordAuraEffectType EffectType = ESwordAuraEffectType::LeftNormal;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartSwordSlashVFX();
	void Multicast_StartSwordSlashVFX_Implementation();


protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UBoxComponent* SlashBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* LeftNormalSlashVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* RightNormalSlashVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* LeftBuffSlashVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attack")
	UNiagaraSystem* RightBuffSlashVFX;



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
	float SwordAuraLifetime = 0.8f;

	UFUNCTION()
	void DestroySwordAura();

	


	//VFX
	UFUNCTION(BlueprintCallable, Category = "Sword FX")
	void StartSwordSlashVFX();

	UFUNCTION(BlueprintCallable, Category = "Sword FX")
	void StopSwordSlashVFX();


	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

};
