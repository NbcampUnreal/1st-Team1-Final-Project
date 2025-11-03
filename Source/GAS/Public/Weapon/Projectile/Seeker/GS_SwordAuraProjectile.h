// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_SwordAuraProjectile.generated.h"

class UBoxComponent;
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UAkAudioEvent;

UENUM(BlueprintType)
enum class ESwordAuraEffectType : uint8
{
	LeftNormal,
	RightNormal,
	LeftBuff,
	RightBuff
};

UENUM(BlueprintType)
enum class ESwordAuraHitTargetType : uint8
{
	Guardian,
	Seeker,
	Character,
	Structure,
	Other
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

	// VFX와 사운드를 하나의 RPC로 통합 (네트워크 최적화)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitEffects(ESwordAuraHitTargetType TargetType, const FVector& HitLocation);
	void Multicast_PlayHitEffects_Implementation(ESwordAuraHitTargetType TargetType, const FVector& HitLocation);


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

	// 타격 시 VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Hit")
	UNiagaraSystem* NormalHitVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Hit")
	UNiagaraSystem* BuffHitVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Hit")
	UNiagaraSystem* NormalBloodSplatterVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Hit")
	UNiagaraSystem* BuffBloodSplatterVFX;

	// VFX 컴포넌트 관리
	UPROPERTY()
	UNiagaraComponent* SlashVFXComponent;

	// 타격 사운드 이벤트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Hit")
	UAkAudioEvent* HitPawnSoundEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Hit")
	UAkAudioEvent* HitSeekerSoundEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Hit")
	UAkAudioEvent* HitStructureSoundEvent;

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
