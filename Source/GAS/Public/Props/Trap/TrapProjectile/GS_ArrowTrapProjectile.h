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

	// Arrow By Sound 관련 변수들
	bool bArrowBySoundPlayed; // Arrow By 사운드가 이미 재생되었는지 체크

	// Arrow By 콜리전 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	USphereComponent* ArrowByCollisionComp;

	// Audio Events - TPS Mode
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|TPS")
	UAkAudioEvent* ImpactSoundEvent_TPS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|TPS")
	UAkAudioEvent* PlayerHitSoundEvent_TPS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|TPS")
	UAkAudioEvent* ArrowBySoundEvent_TPS;

	// Audio Events - RTS Mode
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|RTS")
	UAkAudioEvent* ImpactSoundEvent_RTS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|RTS")
	UAkAudioEvent* PlayerHitSoundEvent_RTS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|RTS")
	UAkAudioEvent* ArrowBySoundEvent_RTS;

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

	/** 현재 RTS 모드인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Sound")
	bool IsRTSMode() const;

	/** 모드에 맞는 사운드 이벤트 선택 */
	UFUNCTION(BlueprintPure, Category = "Sound")
	UAkAudioEvent* SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound) const;

	/** Arrow By 사운드 재생 */
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayArrowBySound();

	/** Arrow By 콜리전 오버랩 이벤트 */
	UFUNCTION()
	void OnArrowByCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	/** Arrow By 콜리전 엔드 오버랩 이벤트 */
	UFUNCTION()
	void OnArrowByCollisionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void PlayHitVFX(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal);

	// 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitEffects(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitVFXOnly(const FVector& ImpactPoint, const FVector& ImpactNormal);
};