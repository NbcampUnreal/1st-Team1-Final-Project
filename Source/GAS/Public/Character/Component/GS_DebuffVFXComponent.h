// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Debuff/EDebuffType.h"
#include "GS_DebuffVFXComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_DebuffVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_DebuffVFXComponent();

	// ======================
	// VFX 설정 (블루프린트)
	// ======================
	
	// 디버프 타입별 VFX 시스템 매핑
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|DebuffSettings")
	TMap<EDebuffType, UNiagaraSystem*> DebuffVFXMap;
	
	// 디버프 타입별 VFX 지속 시간 매핑
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|DebuffSettings")
	TMap<EDebuffType, float> DebuffVFXDurationMap;

	// VFX 스케일 설정 (필요시)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|DebuffSettings")
	TMap<EDebuffType, FVector> DebuffVFXScaleMap;

	// ======================
	// VFX 제어 함수
	// ======================
	
	// 디버프 VFX 재생 (블루프린트 & C++에서 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	void PlayDebuffVFX(EDebuffType DebuffType);
	
	// 디버프 VFX 제거 (블루프린트 & C++에서 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "DebuffVFX")
	void RemoveDebuffVFX(EDebuffType DebuffType);

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|DebuffSettings")
	float DefaultVFXDuration = 5.0f;

private:
	// ======================
	// VFX 재생 (멀티캐스트)
	// ======================
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDebuffVFX(EDebuffType DebuffType, FVector SpawnLocation, FVector Scale);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveDebuffVFX(EDebuffType DebuffType);

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
	
	// 설정된 지속 시간 가져오기
	float GetVFXDuration(EDebuffType DebuffType) const;
	
	// 설정된 VFX 스케일 가져오기
	FVector GetVFXScale(EDebuffType DebuffType) const;
}; 