#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AkGameplayStatics.h"
#include "AkComponent.h"
#include "Engine/TimerHandle.h"
#include "Net/UnrealNetwork.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Engine.h"
#include "GS_AudioComponentBase.generated.h"

class AGS_RTSController;
class AGS_RTSCamera;

/**
 * RTS 커맨드 사운드 타입 (공통 enum)
 */
UENUM(BlueprintType)
enum class ERTSCommandSoundType : uint8
{
    Selection       UMETA(DisplayName = "유닛 선택"),
    Move            UMETA(DisplayName = "이동 명령"),
    Attack          UMETA(DisplayName = "공격 명령"),
    Death           UMETA(DisplayName = "유닛 죽음")
};

/**
 * 오디오 컴포넌트의 공통 기능을 제공하는 베이스 클래스
 * 몬스터와 시커 오디오 컴포넌트가 상속받아 사용
 */
UCLASS(ClassGroup=(Audio), BlueprintType)
class GAS_API UGS_AudioComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_AudioComponentBase();

	// RTPC 이름 상수
	static const FName DistanceToPlayerRTPCName;
	static const FName AttenuationModeRTPCName;
	static const FName OcclusionDisableRTPCName;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// ===================
	// 공통 사운드 설정
	// ===================
	
	// 성능 최적화를 위한 상수들
	static constexpr float DistanceCheckInterval = 1.0f;      // 거리 체크 주기 (1초)
	static constexpr float MinRPCInterval = 0.1f;             // RPC 호출 최소 간격
	static constexpr float MinRTPCUpdateInterval = 0.2f;     // RTPC 업데이트 최소 간격
	static constexpr float RTPCDistanceThreshold = 50.0f;    // RTPC 업데이트를 위한 최소 거리 차이
	
	// 모드별 거리 설정 상수
	static constexpr float RTSMaxDistance = 20000.0f;        // RTS 모드 최대 거리 (200m)
	static constexpr float TPSMaxDistance = 2000.0f;         // TPS 모드 최대 거리 (20m)
	
	// Distance Scaling 설정 상수
	static constexpr float RTSDistanceScaling = 1.0f;        // RTS 모드 (100% = 200m)
	static constexpr float TPSDistanceScaling = 1.0f;        // TPS 모드 (100% = 20m)
	
	// 기타 상수들
	static constexpr float LocalSoundCooldownMultiplier = 0.9f; // 로컬 사운드 쿨다운 배율
	static constexpr int32 MaxActivePlayingIDs = 10;           // 최대 활성 사운드 ID 개수
	static constexpr float DefaultInitTime = -1000.0f;         // 초기 시간 값

protected:
	// ===================
	// 메모리 관리
	// ===================
	
	// 현재 재생 중인 사운드 ID들을 추적
	TArray<AkPlayingID> ActivePlayingIDs;
	
	// 단일 PlayingID (하위 호환성)
	AkPlayingID CurrentPlayingID;
	
	// ===================
	// 네트워크 최적화
	// ===================
	
	// 마지막 RPC 호출 시간 추적
	UPROPERTY(Transient)
	float LastMulticastTime = DefaultInitTime;
	
	// ===================
	// 성능 최적화
	// ===================
	
	// RTPC 업데이트 최적화를 위한 변수들
	UPROPERTY(Transient)
	float LastDistanceRTPCValue = -1.0f;
	
	UPROPERTY(Transient) 
	float LastRTPCUpdateTime = DefaultInitTime;
	
	// ===================
	// 카메라 위치 캐싱
	// ===================
	
	/** 캐싱된 카메라 위치 */
	UPROPERTY(Transient)
	mutable FVector CachedCameraLocation = FVector::ZeroVector;
	
	/** 마지막 카메라 위치 업데이트 시간 */
	UPROPERTY(Transient)
	mutable float LastCameraLocationUpdateTime = DefaultInitTime;
	
	/** 카메라 위치 업데이트 주기 (초) */
	UPROPERTY(Transient)
	mutable float CameraLocationUpdateInterval = 0.1f;
	
	/** 캐싱된 RTS 카메라 액터 */
	UPROPERTY(Transient)
	mutable TWeakObjectPtr<AGS_RTSCamera> CachedRTSCamera;
	
	FTimerHandle DistanceCheckTimerHandle;

public:
	// ===================
	// 공통 인터페이스
	// ===================
	
	/** 현재 RTS 모드인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool IsRTSMode() const;
	
	/** 리스너 위치 가져오기 (RTS/TPS) */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool GetListenerLocation(FVector& OutLocation) const;
	
	/** 모드별 최대 거리 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetMaxDistanceForMode(bool bIsRTS) const;
	
	/** 모드별 Distance Scaling 값 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetDistanceScalingForMode(bool bIsRTS) const;
	
	/** RPC 호출 빈도 체크 */
	bool CanSendRPC() const;
	
	/** RTS 카메라의 실제 보이는 영역 계산 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	FBox2D GetRTSCameraViewBounds() const;
	
	/** 실제 카메라 위치 가져오기 (캐싱 포함) */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool GetActualCameraLocation(FVector& OutLocation) const;
	
	/** 소스가 뷰 프러스텀 내에 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool IsInViewFrustum(const FVector& SourceLocation) const;
	
	/** RTS 오디오 가시성 체크 */
	bool CheckRTSAudioVisibility(AGS_RTSController* RTSController, const FVector& SourceLocation) const;
	
	/** 화면 월드 경계 계산 */
	FBox2D CalculateScreenWorldBounds(AGS_RTSController* RTSController) const;
	
	/** 화면 경계까지의 거리 계산 */
	float CalculateDistanceToScreenBounds(const FVector2D& Point, const FBox2D& Bounds) const;
	
	/** 통로 범위 내에 있는지 확인 */
	bool IsInCorridorRange(const FVector& CameraLocation, const FVector& SourceLocation) const;
	
	// ===================
	// 스프링암 각도 보정 함수들
	// ===================
	
	/** Pitch 각도에 따른 보정 계수 계산 */
	float CalculatePitchCorrectionFactor(float PitchAngle) const;
	
	/** Yaw 각도에 따른 보정 계수 계산 */
	float CalculateYawCorrectionFactor(float YawAngle) const;
	
	/** 스프링암 각도에 따른 위치 보정 적용 */
	FVector2D ApplySpringArmCorrection(
		const FVector2D& OriginalPos, 
		float PitchAngle, 
		float YawAngle, 
		float ArmLength,
		float PitchCorrectionFactor,
		float YawCorrectionFactor) const;
	
	/** 스프링암 각도에 따른 최종 경계 상자 보정 */
	FBox2D ApplyFinalSpringArmCorrection(
		const FBox2D& OriginalBounds, 
		float PitchAngle, 
		float YawAngle, 
		float ArmLength) const;
	
	/** 줌 레벨에 따른 동적 보정 계수 계산 */
	float CalculateZoomCorrectionFactor(float ArmLength) const;

	/** FOV 기반 Pitch 각도 보정 계수 계산 */
	float CalculateFOVPitchCorrection(float PitchAngle, float CameraHeight) const;

protected:
	// ===================
	// 메모리 관리 헬퍼
	// ===================
	
	/** 활성 사운드 정리 */
	void CleanupFinishedSounds();
	
	/** 모든 활성 사운드 중지 */
	void StopAllActiveSounds();
	
	/** 새 사운드 ID 등록 */
	void RegisterPlayingID(AkPlayingID NewPlayingID);

	// ===================
	// 거리 및 RTPC 관리
	// ===================
	
	/** 거리 및 상태 체크 (타이머 콜백) */
	virtual void UpdateDistanceRTPC();
	
	/** RTPC 업데이트가 필요한지 확인 */
	bool ShouldUpdateRTPC(float NewDistance, float CurrentTime) const;
	
	/** Distance Scaling 설정 */
	void SetDistanceScaling(bool bIsRTS);

	// ===================
	// 가상 함수 (하위 클래스에서 구현)
	// ===================
	
	/** 거리 기반 상태 변경 체크 (하위 클래스에서 구현) */
	virtual void CheckForStateChanges() {}
	
	/** 최대 오디오 거리 반환 (하위 클래스에서 구현) */
	virtual float GetMaxAudioDistance() const { return TPSMaxDistance; }

public:
	// Replication 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};