#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Engine/HitResult.h"
#include "GS_ArrowTrapProjectile.generated.h"

class UGS_ProjectilePoolComp;
class UAkAudioEvent;
class UNiagaraSystem;

// 히트 타입 열거형
UENUM(BlueprintType)
enum class EArrowHitType : uint8
{
	Wall		UMETA(DisplayName = "Wall"),
	Player		UMETA(DisplayName = "Player"),
	Other		UMETA(DisplayName = "Other")
};

UCLASS()
class GAS_API AGS_ArrowTrapProjectile : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_ArrowTrapProjectile();

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	AGS_NonTrigTrapBase* OwningTrap;

	UPROPERTY()
	TObjectPtr<UGS_ProjectilePoolComp> OwningPool;

	FTimerHandle LifeSpanHandle;
	// Audio Events - Wwise
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	UAkAudioEvent* ImpactSoundEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	UAkAudioEvent* PlayerHitSoundEvent;

	// VFX Systems
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* ImpactVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* PlayerHitVFX;

	UFUNCTION(BlueprintCallable, Category = "Trap")
	void Init(AGS_NonTrigTrapBase* InTrap);

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void ActivateProjectile(const FVector& SpawnLocation, const FRotator& Rotation, float Speed);
	
	UFUNCTION(BlueprintCallable)
	void DeactivateProjectile();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnActivateEffect();
	void OnActivateEffect_Implementation();

	bool IsReady() const;

	void StickWithVisualOnly(const FHitResult& Hit);
	
	void OnLifeSpanExpired();

protected:
	virtual void BeginPlay() override;

	// 히트 타입 결정
	UFUNCTION(BlueprintCallable, Category = "Trap")
	EArrowHitType DetermineHitType(AActor* HitActor, const FHitResult& Hit) const;

	UFUNCTION(BlueprintCallable, Category = "Trap")
	void HandleHitEffects(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal);

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayHitSound(EArrowHitType HitType, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void PlayHitVFX(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal);

	// 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitEffects(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitVFXOnly(const FVector& ImpactPoint, const FVector& ImpactNormal);
};