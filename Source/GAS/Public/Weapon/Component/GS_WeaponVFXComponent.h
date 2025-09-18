// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "GS_WeaponVFXComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// 무기 VFX 타입 정의
UENUM(BlueprintType)
enum class EWeaponVFXType : uint8
{
	HitAura			UMETA(DisplayName = "Hit Aura"),		// 타격 시 아우라
	Trail			UMETA(DisplayName = "Trail"),			// 무기 궤적
	Charge			UMETA(DisplayName = "Charge"),			// 차징 이펙트
	SpecialAttack	UMETA(DisplayName = "Special Attack"),	// 특수 공격
	Enchant			UMETA(DisplayName = "Enchant"),			// 인챈트 효과
	Slash			UMETA(DisplayName = "Slash"),			// 베기 이펙트
	GuardSuccess	UMETA(DisplayName = "Guard Success")	// 방어 성공 이펙트
};

// 시커 타입별 아우라 이펙트 정의
UENUM(BlueprintType)
enum class ESeekerAuraType : uint8
{
	Chan		UMETA(DisplayName = "Chan"),
	Ares		UMETA(DisplayName = "Ares"), 
	Merci		UMETA(DisplayName = "Merci"),
	Default		UMETA(DisplayName = "Default")
};

// VFX 설정 구조체
USTRUCT(BlueprintType)
struct FWeaponVFXSeekerSettings
{
	GENERATED_BODY()

	// 시커 타입별 VFX 시스템
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<ESeekerAuraType, UNiagaraSystem*> VFXSystemMap;

	// 시커 타입별 VFX 지속 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<ESeekerAuraType, float> VFXDurationMap;

	// 시커 타입별 VFX 스케일
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<ESeekerAuraType, FVector> VFXScaleMap;

	// 시커 타입별 VFX 위치 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<ESeekerAuraType, FVector> VFXLocationOffsetMap;

	// 시커 타입별 VFX 회전 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<ESeekerAuraType, FRotator> VFXRotationOffsetMap;
};

// 무기 VFX 설정을 위한 Data Asset
UCLASS(BlueprintType)
class GAS_API UGS_WeaponVFXDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// VFX 타입별 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<EWeaponVFXType, FWeaponVFXSeekerSettings> VFXSettingsMap;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_WeaponVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_WeaponVFXComponent();
	
	// 공통 VFX 설정 Data Asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Settings")
	UGS_WeaponVFXDataAsset* WeaponVFXSettings;
	
	// 개별 VFX 오버라이드 (특정 무기만 다른 VFX 사용시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Override")
	TMap<EWeaponVFXType, FWeaponVFXSeekerSettings> OverrideVFXSettingsMap;

	// 기본 VFX 지속시간 (설정되지 않은 경우)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Settings")
	float DefaultVFXDuration = 3.0f;

	// 무기에 붙일 소켓 이름 (비어있으면 Root에 붙음)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Settings")
	FName AttachSocketName = NAME_None;
	
	// 아우라 VFX 활성화 (히트 감지 시 호출)
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void ActivateHitAura(ESeekerAuraType SeekerType);
	
	// 아우라 VFX 비활성화
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void DeactivateHitAura();

	// 아우라가 활성화되어 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	bool IsHitAuraActive() const;

	// 슬래시 VFX 재생 (충돌 감지 시 호출)
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void PlaySlashVFX(const FHitResult& HitResult, ESeekerAuraType AttackerSeekerType);
	
	// ======================
	// 확장 VFX 기능들
	// ======================
	
	// 무기 트레일 이펙트
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void ActivateTrailVFX(bool bActivate = true);

	// 차징 이펙트
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void ActivateChargeVFX(float ChargeLevel = 1.0f);

	// 특수 공격 이펙트
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void PlaySpecialAttackVFX(ESeekerAuraType SeekerType);

	// 인챈트 이펙트
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void ActivateEnchantVFX(ESeekerAuraType SeekerType, float Duration = -1.0f);

	// 가드 성공 이펙트
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void PlayGuardSuccessVFX(const FHitResult& HitResult, ESeekerAuraType DefenderSeekerType);

	// 모든 VFX 정리
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	void ClearAllVFX();

	// 현재 아우라 타입 가져오기
	UFUNCTION(BlueprintCallable, Category = "WeaponVFX")
	ESeekerAuraType GetCurrentAuraType() const { return CurrentAuraType; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ======================
	// VFX 가져오기 헬퍼 함수
	// ======================
	
	// VFX 시스템 가져오기 (오버라이드 우선, 없으면 공통 설정)
	UNiagaraSystem* GetWeaponVFX(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const;
	float GetVFXDuration(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const;
	FVector GetVFXScale(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const;
	FVector GetVFXLocationOffset(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const;
	FRotator GetVFXRotationOffset(EWeaponVFXType VFXType, ESeekerAuraType SeekerType) const;

	// ======================
	// VFX 재생 (멀티캐스트)
	// ======================
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateHitAura(ESeekerAuraType SeekerType, FVector LocationOffset, FRotator RotationOffset, FVector Scale, float Duration);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DeactivateHitAura();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySlashVFX(FVector ImpactPoint, FVector WeaponVelocity, ESeekerAuraType SeekerType);
	
	// 확장 VFX용 멀티캐스트 함수들
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateTrailVFX(bool bActivate, ESeekerAuraType SeekerType);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateChargeVFX(float ChargeLevel, ESeekerAuraType SeekerType);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpecialAttackVFX(ESeekerAuraType SeekerType);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivateEnchantVFX(ESeekerAuraType SeekerType, float Duration);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayGuardSuccessVFX(FVector ImpactPoint, FVector ImpactNormal, ESeekerAuraType DefenderSeekerType);

	// ======================
	// VFX 관리 시스템
	// ======================
	
	// 현재 재생 중인 VFX 컴포넌트들
	UPROPERTY()
	UNiagaraComponent* ActiveHitAuraVFXComponent;
	
	UPROPERTY()
	UNiagaraComponent* TrailVFXComponent;
	
	UPROPERTY()
	UNiagaraComponent* ChargeVFXComponent;
	
	UPROPERTY()
	UNiagaraComponent* EnchantVFXComponent;
	
	// 현재 아우라 타입
	ESeekerAuraType CurrentAuraType;

	// VFX 자동 제거 타이머들
	FTimerHandle HitAuraTimerHandle;
	FTimerHandle HitAuraCleanupTimerHandle;
	FTimerHandle EnchantTimerHandle;
	FTimerHandle EnchantCleanupTimerHandle;
	
	// VFX 자동 제거 타이머 콜백
	void DeactivateHitAuraTimerCallback();
	void CleanupHitAuraTimerCallback();
	void DeactivateEnchantTimerCallback();
	void CleanupEnchantTimerCallback();
	
	// VFX 컴포넌트 상태 관리
	bool bHitAuraDeactivating;
	bool bEnchantDeactivating;
	
	// 안전한 VFX 컴포넌트 정리
	void CleanupHitAuraVFXComponent();
	void CleanupTrailVFXComponent();
	void CleanupChargeVFXComponent();
	void CleanupEnchantVFXComponent();
	
	// 부드러운 VFX 비활성화
	void SoftDeactivateHitAura();
	void SoftDeactivateEnchant();

	// 무기 메시 컴포넌트 가져오기
	USceneComponent* GetWeaponMeshComponent() const;

	// 레벨 전환 시 안전성 검사
	bool IsValidForVFXOperation() const;
	
	// 현재 소유자의 시커 타입 자동 감지
	ESeekerAuraType GetOwnerSeekerType() const;
};