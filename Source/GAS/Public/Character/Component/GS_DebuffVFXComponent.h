// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Debuff/EDebuffType.h"
#include "Engine/DataAsset.h"
#include "GS_DebuffVFXComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// 공통 디버프 VFX 설정을 위한 Data Asset
UCLASS(BlueprintType)
class GAS_API UGS_DebuffVFXDataAsset : public UPrimaryDataAsset
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_DebuffVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_DebuffVFXComponent();

	// ======================
	// VFX 설정 (블루프린트)
	// ======================
	
	// 공통 VFX 설정 Data Asset (모든 캐릭터가 공유)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Settings")
	UGS_DebuffVFXDataAsset* CommonVFXSettings;
	
	// 개별 VFX 오버라이드 (특정 캐릭터만 다른 VFX 사용시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Override", meta = (ToolTip = "공통 설정을 오버라이드할 개별 VFX 설정"))
	TMap<EDebuffType, UNiagaraSystem*> OverrideDebuffVFXMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Override")
	TMap<EDebuffType, UNiagaraSystem*> OverrideDebuffExpireVFXMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Override")
	TMap<EDebuffType, float> OverrideDebuffVFXDurationMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Override")
	TMap<EDebuffType, FVector> OverrideDebuffVFXScaleMap;

	// ======================
	// VFX 제어 함수
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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 디폴트 VFX 지속 시간 (맵에 설정되지 않은 경우)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Settings")
	float DefaultVFXDuration = 5.0f;

private:
	// ======================
	// VFX 가져오기 헬퍼 함수
	// ======================
	
	// VFX 시스템 가져오기 (오버라이드 우선, 없으면 공통 설정)
	UNiagaraSystem* GetDebuffVFX(EDebuffType DebuffType) const;
	UNiagaraSystem* GetDebuffExpireVFX(EDebuffType DebuffType) const;
	
	// ======================
	// VFX 재생 (멀티캐스트)
	// ======================
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDebuffVFX(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveDebuffVFX(EDebuffType DebuffType);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDebuffExpireVFX(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale);

	// ======================
	// VFX 관리 시스템
	// ======================
	
	// 현재 재생 중인 VFX 컴포넌트들
	UPROPERTY()
	TMap<EDebuffType, UNiagaraComponent*> ActiveVFXComponents;
	
	// VFX 제거 타이머들
	TMap<EDebuffType, FTimerHandle> VFXTimerHandles;
	
	// VFX 제거 타이머 콜백
	void RemoveVFXTimerCallback(EDebuffType DebuffType);
	
	// 설정된 지속 시간 가져오기 (오버라이드 우선, 없으면 공통 설정)
	float GetVFXDuration(EDebuffType DebuffType) const;
	
	// 설정된 VFX 스케일 가져오기 (오버라이드 우선, 없으면 공통 설정)
	FVector GetVFXScale(EDebuffType DebuffType) const;
}; 