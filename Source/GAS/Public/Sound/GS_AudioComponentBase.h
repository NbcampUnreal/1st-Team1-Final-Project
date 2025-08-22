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
	static constexpr float RTSMaxDistance = 15000.0f;        // RTS 모드 최대 거리 (150m) - 더 넓게 설정
	static constexpr float TPSMaxDistance = 2000.0f;         // TPS 모드 최대 거리 (20m)
	
	// Distance Scaling 설정 상수
	static constexpr float RTSDistanceScaling = 1.0f;        // RTS 모드 (800% = 150m) - 더 강하게 설정
	static constexpr float TPSDistanceScaling = 0.0f;        // TPS 모드 (100% = 20m)
	
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