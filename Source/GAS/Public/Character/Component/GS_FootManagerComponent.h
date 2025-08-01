// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "Components/DecalComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "NiagaraSystem.h"
#include "AkGameplayTypes.h"
#include "GS_FootManagerComponent.generated.h"

// Forward Declarations
class UNiagaraSystem;
class UNiagaraComponent;
class UAkAudioEvent;
class UMaterialInterface;

/**
 * 발걸음 타입을 정의하는 열거형
 */
UENUM(BlueprintType)
enum class EFootStep : uint8
{
	LeftFoot	UMETA(DisplayName = "Left Foot"),
	RightFoot	UMETA(DisplayName = "Right Foot")
};

/**
 * 발 감지 방식 열거형
 */
UENUM(BlueprintType)
enum class EFootDetectionMethod : uint8
{
	VelocityBased	UMETA(DisplayName = "Velocity Based"),
	HeightBased		UMETA(DisplayName = "Height Based"),
	Alternating		UMETA(DisplayName = "Alternating")
};

/**
 * 발걸음 시스템을 관리하는 컴포넌트
 * 라인 트레이스, 데칼 스폰, 사운드 재생, VFX 효과를 통합 관리
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_FootManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGS_FootManagerComponent();

	/**
	 * 발걸음 처리 메인 함수 - 애님 노티파이에서 호출
	 * @param Foot 발걸음 타입 (왼발/오른발)
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep")
	void HandleFootstep(EFootStep Foot);

	/**
	 * 특정 Niagara 시스템으로 발자국 먼지 효과를 강제.
	 * @param VFXSystem 오버라이드할 Niagara 시스템. nullptr을 전달하면 오버라이드가 해제.
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep")
	void OverrideFootDustEffect(UNiagaraSystem* VFXSystem);

	/**
	 * 발자국 먼지 효과 오버라이드를 해제하고 원래 로직으로 되돌림림.
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep")
	void ClearFootDustEffectOverride();

	/**
	 * FoleyEvent와 호환되는 자동 발걸음 처리 함수
	 * 발의 위치와 속도를 분석하여 자동으로 발걸음 감지
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep")
	void HandleFoleyEvent();

	/**
	 * 수동으로 자동 감지 모드 토글
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep")
	void ToggleAutoDetection(bool bEnable) { bAutoDetectionEnabled = bEnable; }

	/**
	 * 물 소리 테스트용 함수 - 강제로 물 표면 발걸음 재생
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep|Debug")
	void TestWaterFootstep();

	/**
	 * 현재 서 있는 표면 타입을 반환하는 디버그 함수
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep|Debug")
	int32 GetCurrentSurfaceType();

	/**
	 * 물 관련 나이아가라 VFX 동적 설정 함수
	 * @param EffectType 설정할 이펙트 타입 (0: Splash, 1: DeepWater, 2: Ripple, 3: Bubble, 4: Mist)
	 * @param NiagaraSystem 설정할 나이아가라 시스템
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep|Water VFX")
	void SetWaterVFX(int32 EffectType, UNiagaraSystem* NiagaraSystem);

	/**
	 * 물 깊이에 따른 적절한 이펙트 선택
	 * @param WaterDepth 물 깊이 (cm)
	 * @return 선택된 나이아가라 시스템
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep|Water VFX")
	UNiagaraSystem* GetWaterEffectByDepth(float WaterDepth);

	/**
	 * 모든 물 이펙트를 한번에 스폰 (조합 효과)
	 * @param Location 스폰 위치
	 * @param WaterDepth 물 깊이
	 */
	UFUNCTION(BlueprintCallable, Category = "Footstep|Water VFX")
	void SpawnCombinedWaterEffects(const FVector& Location, float WaterDepth = 10.0f);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// ======== Configuration Properties ========

	/** 라인 트레이스 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Trace", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float TraceDistance = 50.0f;

	/** 트레이스에 사용할 충돌 채널 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

	/** 발 데칼 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Decal", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	FVector FootDecalSize = FVector(10.0f, 5.0f, 5.0f);

	/** 발 데칼 수명 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Decal", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	float FootDecalLifeSpan = 15.0f;

	/** 자동 발걸음 감지 모드 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	bool bAutoDetectionEnabled = true;

	/** 발자국 감지를 위한 최소 이동 속도 임계값 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float MinimumMovementSpeed;

	/** 발자국 데칼 회전 오프셋 (도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Decal", meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	float DecalRotationOffsetYaw = 0.0f;

	// ======== Asset Maps ========

	/** 왼발 데칼 머티리얼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Assets")
	UMaterialInterface* LeftFootDecal;

	/** 오른발 데칼 머티리얼 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Assets")
	UMaterialInterface* RightFootDecal;

	/** 발걸음 사운드 이벤트 (Wwise 스위치 컨테이너 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Assets")
	UAkAudioEvent* FootstepSoundEvent;

	/** Wwise 스위치 그룹 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Assets")
	FString SwitchGroupName = TEXT("Surface_Type");

	/** 물리 표면별 Wwise 스위치 값 맵 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Assets")
	TMap<TEnumAsByte<EPhysicalSurface>, FString> SurfaceSwitchValues;

	/** 물리 표면별 먼지 VFX 맵 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Assets")
	TMap<TEnumAsByte<EPhysicalSurface>, UNiagaraSystem*> FootDustEffects;

	/** 물 표면에서의 특별한 처리 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water")
	bool bEnableWaterEffects = true;

	/** 물 표면에서의 물보라 크기 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float WaterSplashScale = 1.0f;

	/** 물 관련 나이아가라 VFX 시스템들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX")
	UNiagaraSystem* WaterSplashEffect;

	/** 깊은 물에서의 나이아가라 VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX")
	UNiagaraSystem* DeepWaterSplashEffect;

	/** 물 파동 나이아가라 VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX")
	UNiagaraSystem* WaterRippleEffect;

	/** 물 거품 나이아가라 VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX")
	UNiagaraSystem* WaterBubbleEffect;

	/** 물 안개 나이아가라 VFX */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX")
	UNiagaraSystem* WaterMistEffect;

	/** 물 깊이에 따른 이펙트 전환 임계값 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float DeepWaterThreshold = 20.0f;

	/** 물 이펙트 동시 재생 개수 제한 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep|Water VFX", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MaxConcurrentWaterEffects = 3;

	// ======== Blueprint Events  ========

	/** 발소리가 재생되었을 때 호출되는 델리게이트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFootSoundPlayedDelegate, EPhysicalSurface, Surface, FVector, Location);
	
	UPROPERTY(BlueprintAssignable, Category = "Footstep|Events")
	FOnFootSoundPlayedDelegate OnFootSoundPlayed;

	/** 데칼이 스폰되었을 때 호출되는 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Footstep|Events")
	void OnFootDecalSpawned(UDecalComponent* DecalComponent, EFootStep Foot);

	/** 먼지 VFX가 스폰되었을 때 호출되는 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Footstep|Events")
	void OnFootDustSpawned(EPhysicalSurface Surface, const FVector& Location);

	/** 발자국 간 최소 시간 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float FootstepCooldownTime;

	/** 마지막 발자국으로부터 최소 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float MinimumFootstepDistance;

	/** 마지막 발자국 시간 */
	float LastFootstepTime;

	/** 마지막 발자국 위치 */
	FVector LastFootstepLocation;

private:
	// ======== Private Functions ========

	/**
	 * 발 소켓에서 라인 트레이스를 수행
	 * @param Foot 발걸음 타입
	 * @param OutHitResult 충돌 결과
	 * @return 트레이스 성공 여부
	 */
	bool PerformFootTrace(EFootStep Foot, FHitResult& OutHitResult);

	/**
	 * 발걸음 데칼을 스폰
	 * @param Surface 물리 표면
	 * @param Location 스폰 위치
	 * @param Normal 표면 법선
	 * @param Foot 발걸음 타입
	 */
	void SpawnFootstepDecal(EPhysicalSurface Surface, const FVector& Location, const FVector& Normal, EFootStep Foot);

	/**
	 * 발걸음 사운드를 재생
	 * @param Surface 물리 표면
	 * @param Location 재생 위치
	 */
	void PlayFootstepSound(EPhysicalSurface Surface, const FVector& Location);

	/**
	 * 먼지 VFX를 스폰
	 * @param Surface 물리 표면
	 * @param Location 스폰 위치
	 * @param Normal 표면 법선
	 */
	void SpawnFootDustEffect(EPhysicalSurface Surface, const FVector& Location, const FVector& Normal);

	/**
	 * 발 타입에 따른 소켓 이름을 반환
	 * @param Foot 발걸음 타입
	 * @return 소켓 이름
	 */
	FName GetFootSocketName(EFootStep Foot) const;

	/**
	 * 소유자의 스켈레탈 메시 컴포넌트를 가져옴
	 * @return 스켈레탈 메시 컴포넌트 (없으면 nullptr)
	 */
	USkeletalMeshComponent* GetOwnerSkeletalMesh() const;

	/**
	 * 발의 위치와 속도를 분석하여 현재 활성화된 발걸음을 감지
	 * @return 감지된 발걸음 타입
	 */
	EFootStep DetectActiveFootstep();

	// ======== Cached Components ========

	/** 캐시된 스켈레탈 메시 컴포넌트 */
	UPROPERTY()
	USkeletalMeshComponent* CachedSkeletalMesh;

	/** 오버라이드된 발자국 먼지 VFX */
	UPROPERTY()
	UNiagaraSystem* OverriddenFootDustEffect;

	// ======== Socket Names ========

	/** 왼발 소켓 이름 */
	static const FName LeftFootSocketName;

	/** 오른발 소켓 이름 */
	static const FName RightFootSocketName;
}; 