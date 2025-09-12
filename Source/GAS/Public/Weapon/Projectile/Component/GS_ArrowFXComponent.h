// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/HitResult.h"
#include "AkAudioEvent.h"
#include "Weapon/Projectile/GS_TargetType.h"
#include "Weapon/Projectile/Seeker/GS_ArrowType.h"
#include "GS_ArrowFXComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_ArrowFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_ArrowFXComponent();

	// 화살 트레일 VFX 시작 (화살 타입별)
	UFUNCTION(BlueprintCallable, Category = "Arrow FX")
	void StartArrowTrailVFX(EArrowType ArrowType);

	// 화살 트레일 VFX 중지
	UFUNCTION(BlueprintCallable, Category = "Arrow FX")
	void StopArrowTrailVFX();

	// Hit VFX 재생
	UFUNCTION(BlueprintCallable, Category = "Arrow FX")
	void PlayHitVFX(ETargetType TargetType, const FHitResult& SweepResult);

	// Hit Sound 재생
	UFUNCTION(BlueprintCallable, Category = "Arrow FX")
	void PlayHitSound(ETargetType TargetType, const FHitResult& SweepResult);

	// 화살 타입 설정 (외부에서 호출용)
	UFUNCTION(BlueprintCallable, Category = "Arrow FX")
	void SetArrowType(EArrowType ArrowType);

	// 멀티캐스트 함수들 - Trail VFX
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartArrowTrailVFX(EArrowType ArrowType);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopArrowTrailVFX();

	// 멀티캐스트 함수들 - Hit VFX
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitVFX(ETargetType TargetType, const FHitResult& SweepResult);

	// 멀티캐스트 함수들 - Hit Sound
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitSound(ETargetType TargetType, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 화살 트레일 VFX 나이아가라 시스템 (타입별)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail VFX")
	UNiagaraSystem* NormalArrowTrailVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail VFX")
	UNiagaraSystem* AxeArrowTrailVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trail VFX")
	UNiagaraSystem* ChildArrowTrailVFX;

	// 현재 활성화된 트레일 VFX 컴포넌트
	UPROPERTY()
	UNiagaraComponent* ActiveTrailVFXComponent;

	// VFX 위치 및 회전 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FVector TrailVFXLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FRotator TrailVFXRotationOffset = FRotator::ZeroRotator;

	// VFX 스케일
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	FVector TrailVFXScale = FVector::OneVector;

	// 히트 VFX 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitPawnVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitStructureVFX;

	// Wwise 히트 사운드 이벤트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	UAkAudioEvent* HitPawnSoundEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	UAkAudioEvent* HitStructureSoundEvent;

private:
	// 컴포넌트가 부착될 소유자 액터
	UPROPERTY()
	AActor* OwnerActor;
}; 