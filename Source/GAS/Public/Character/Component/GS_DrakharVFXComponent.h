#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "Character/Debuff/EDebuffType.h"
#include "Character/Component/GS_VFXComponent.h"
#include "Engine/DataAsset.h"
#include "GS_DrakharVFXComponent.generated.h"

class AGS_Drakhar;
class UNiagaraSystem;
class UNiagaraComponent;
class UArrowComponent;
class UGS_CameraShakeComponent;
class UGS_FootManagerComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

// Drakhar 디버프 VFX 설정을 위한 Data Asset
UCLASS(BlueprintType)
class GAS_API UGS_DrakharDebuffVFXDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 디버프 타입별 VFX 시스템 매핑 (디버프 적용 시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<EDebuffType, UNiagaraSystem*> DebuffVFXMap;

	// 디버프 타입별 만료 VFX 시스템 매핑 (디버프 해제 시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<EDebuffType, UNiagaraSystem*> DebuffExpireVFXMap;

	// 디버프 타입별 VFX 지속 시간 매핑
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<EDebuffType, float> DebuffVFXDurationMap;

	// VFX 스케일 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<EDebuffType, FVector> DebuffVFXScaleMap;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_DrakharVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_DrakharVFXComponent();

	// 파라미터명 상수화
	static const FName Param_DashDirection;
	static const FName Param_DashSpeed;
	static const FName Param_Scale;
	static const FName Param_CrackIntensity;
	static const FName Param_CrackRadius;
	static const FName Param_DustIntensity;
	static const FName Param_DustRadius;
	static const FName Param_WindStrength;

	// ======================
	// 디버프 VFX 제어 함수
	// ======================

	// 디버프 VFX 재생
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	void PlayDebuffVFX(EDebuffType DebuffType);

	// 디버프 VFX 제거
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	void RemoveDebuffVFX(EDebuffType DebuffType);

	// 디버프 만료 VFX 재생 (디버프가 끝날 때 특별한 효과)
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	void PlayDebuffExpireVFX(EDebuffType DebuffType);

	// 모든 디버프 VFX 제거
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	void RemoveAllDebuffVFX();

	// 특정 디버프 VFX가 재생 중인지 확인
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	bool IsDebuffVFXActive(EDebuffType DebuffType) const;

protected:
	// 디버프 VFX 설정 Data Asset
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Settings")
	UGS_DebuffVFXDataAsset* DebuffVFXSettings;

	// 개별 디버프 VFX 오버라이드 (특정 디버프만 다른 VFX 사용시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Override", meta = (ToolTip = "공통 설정을 오버라이드할 개별 디버프 VFX 설정"))
	TMap<EDebuffType, UNiagaraSystem*> OverrideDebuffVFXMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Override")
	TMap<EDebuffType, UNiagaraSystem*> OverrideDebuffExpireVFXMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Override")
	TMap<EDebuffType, float> OverrideDebuffVFXDurationMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Override")
	TMap<EDebuffType, FVector> OverrideDebuffVFXScaleMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Override", meta = (ToolTip = "디버프 VFX의 스폰 위치 오프셋 (Z축으로 높이 조절 가능)"))
	TMap<EDebuffType, FVector> OverrideDebuffVFXOffsetMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Debuff|Override", meta = (ToolTip = "디버프 만료 VFX의 스폰 위치 오프셋 (Z축으로 높이 조절 가능)"))
	TMap<EDebuffType, FVector> OverrideDebuffExpireVFXOffsetMap;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveWingRushVFXComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveDustVFXComponent;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveGroundCrackVFXComponent;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveDustCloudVFXComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveFlyingDustVFXComponent;
	
	void OnFlyStart();
	void OnFlyEnd();

	void OnUltimateStart();
	void OnEarthquakeStart();

	void StartWingRushVFX();
	void StopWingRushVFX();

	void StartDustVFX();
	void StopDustVFX();
	
	void StartGroundCrackVFX();
	void StopGroundCrackVFX();

	void StartDustCloudVFX();
	void StopDustCloudVFX();
	
	void PlayAttackHitVFX(FVector ImpactPoint);
	void PlayEarthquakeImpactVFX(const FVector& ImpactLocation);
	void PlayFeverEarthquakeImpactVFX(const FVector& ImpactLocation);

	void HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter);

	void OnFeverModeChanged(bool bIsFeverMode);

	void ApplyFeverModeOverlay();
	void RemoveFeverModeOverlay();

protected:
	UPROPERTY()
	TObjectPtr<UGS_FootManagerComponent> FootManagerComponent;

	// Flying Dust VFX 위치 업데이트 타이머
	FTimerHandle FlyingDustUpdateTimerHandle;

	// 타이머/클린업 지연
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Timing", meta=(ClampMin="0.01"))
	float FlyingDustUpdateInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Timing", meta=(ClampMin="0.01"))
	float WingRushCleanupDelay = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Timing", meta=(ClampMin="0.01"))
	float DustCleanupDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Timing", meta=(ClampMin="0.01"))
	float GroundCrackCleanupDelay = 3.0f;

	// Flying Dust VFX 위치 업데이트 (Timer 콜백)
	void UpdateFlyingDustVFXLocation();
	
	UPROPERTY()
	TObjectPtr<UArrowComponent> WingRushVFXSpawnPoint;
	
	UPROPERTY()
	TObjectPtr<UArrowComponent> EarthquakeVFXSpawnPoint;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FeverModeOverlayMID;

	// ======================
	// 디버프 VFX 관리 시스템
	// ======================

	// 현재 재생 중인 디버프 VFX 컴포넌트들
	UPROPERTY()
	TMap<EDebuffType, UNiagaraComponent*> ActiveDebuffVFXComponents;

	// 디버프 VFX 제거 타이머들
	TMap<EDebuffType, FTimerHandle> DebuffVFXTimerHandles;

	// 현재 제거할 디버프 타입 (타이머 콜백용)
	EDebuffType CurrentDebuffToRemove;

	// 디버프 VFX 제거 타이머 콜백
	void RemoveDebuffVFXTimerCallback();

	// 설정된 지속 시간 가져오기 (오버라이드 우선, 없으면 공통 설정)
	float GetDebuffVFXDuration(EDebuffType DebuffType) const;

	// 설정된 디버프 VFX 스케일 가져오기 (오버라이드 우선, 없으면 공통 설정)
	FVector GetDebuffVFXScale(EDebuffType DebuffType) const;

	// 설정된 디버프 VFX 오프셋 가져오기 (오버라이드 우선, 없으면 공통 설정)
	FVector GetDebuffVFXOffset(EDebuffType DebuffType) const;

	// 설정된 디버프 만료 VFX 오프셋 가져오기 (오버라이드 우선, 없으면 공통 설정)
	FVector GetDebuffExpireVFXOffset(EDebuffType DebuffType) const;

	// 디버프 VFX 시스템 가져오기 (오버라이드 우선, 없으면 공통 설정)
	UNiagaraSystem* GetDebuffVFX(EDebuffType DebuffType) const;
	UNiagaraSystem* GetDebuffExpireVFX(EDebuffType DebuffType) const;

	// 디버프 VFX 재생 (멀티캐스트)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDebuffVFX(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveDebuffVFX(EDebuffType DebuffType);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDebuffExpireVFX(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale);

private:
	UPROPERTY()
	AGS_Drakhar* OwnerDrakhar;
};
