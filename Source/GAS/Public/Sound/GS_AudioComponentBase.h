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
class AGS_RoomBase;
class UAkComponent;

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

	// RTPC 포인터 (UAkRtpc* 기반 통일)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|RTPC")
	UAkRtpc* DistanceToPlayerRTPC = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|RTPC")  
	UAkRtpc* AttenuationModeRTPC = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|RTPC")
	UAkRtpc* OcclusionDisableRTPC = nullptr;

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
	
	// Distance Scaling 설정 상수 (개선된 RTS/TPS 구분)
	static constexpr float RTSDistanceScaling = 2.0f;        // RTS 모드 (200% = 400m)
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
	
	/** 현재 RTS 모드인지 여부를 캐싱하여 중복 호출 방지 */
	bool bIsRTSModeCached;

	/** 오디오 컴포넌트 초기화 여부 플래그 */
	bool bIsAudioComponentInitialized;
	
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
	
	/** 실제 카메라 위치 가져오기 (캐싱 포함) */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool GetActualCameraLocation(FVector& OutLocation) const;
	
	/** 소스가 뷰 프러스텀 내에 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool IsInViewFrustum(const FVector& SourceLocation) const;
	
	/** 화면 투영 기반 소스 가시성 체크 */
	bool IsSourceVisibleOnScreen(AGS_RTSController* RTSController, const FVector& SourceLocation) const;
	
	/** 두 위치가 같은 방에 있는지 확인 */
	bool IsInSameRoom(const FVector& ListenerPos, const FVector& SourcePos) const;

	/** 두 방이 연결되어 있는지 확인 */
	bool AreRoomsConnected(AGS_RoomBase* Room1, AGS_RoomBase* Room2) const;

	/**
	 * 오디오 시스템 검증 (로컬 사운드 재생용)
	 *
	 * 데디케이티드 서버 및 Wwise 초기화 상태를 체크합니다.
	 * 로컬 사운드 재생 전에 호출하여 재생 가능 여부를 판단합니다.
	 *
	 * @return 오디오를 재생해야 하면 true, 그렇지 않으면 false
	 */
	UFUNCTION(BlueprintPure, Category = "Audio")
	bool IsAudioSystemValid() const;


protected:
	/** 특정 위치에 있는 Room을 찾는 함수 */
	AGS_RoomBase* FindRoomAtLocation(const FVector& Location) const;

	// ==========================
	// Multicast RPC 최적화 헬퍼
	// ==========================

	/**
	 * Multicast RPC에서 사운드 재생 가능 여부를 종합적으로 체크하는 헬퍼 함수
	 * 
	 * @param SourceActor 사운드 발생 액터
	 * @param OutIsRTSMode RTS 모드 여부 (출력)
	 * @param OutListenerLocation 리스너 위치 (출력)
	 * @param bSkipViewFrustumCheck ViewFrustum 체크를 건너뛸지 여부 (피격/죽음 사운드 등)
	 * @return 사운드를 재생해야 하면 true, 그렇지 않으면 false
	 */
	bool ShouldPlayMulticastSound(AActor* SourceActor, bool& OutIsRTSMode, FVector& OutListenerLocation, bool bSkipViewFrustumCheck = false) const;

	/**
	 * Multicast RPC 사운드 재생 전처리 (간소화 버전)
	 * Distance Scaling을 자동으로 설정하고 재생 가능 여부만 반환
	 * 
	 * @param SourceActor 사운드 발생 액터
	 * @param bSkipViewFrustumCheck ViewFrustum 체크를 건너뛸지 여부
	 * @return 사운드를 재생해야 하면 true
	 */
	bool PrepareMulticastSound(AActor* SourceActor, bool bSkipViewFrustumCheck = false);

	/**
	 * 모드별 사운드 이벤트 선택 (TPS/RTS 자동 폴백)
	 * 
	 * @param TPSSound TPS 모드 사운드
	 * @param RTSSound RTS 모드 사운드
	 * @param bUseRTSMode 강제로 RTS 모드 사용 (기본값은 자동 감지)
	 * @return 선택된 사운드 이벤트 (RTS가 없으면 TPS로 폴백)
	 */
	UAkAudioEvent* SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound, bool bUseRTSMode = false) const;

	// ===============
	// 메모리 관리 헬퍼
	// ===============
	
	/** 활성 사운드 정리 */
	void CleanupFinishedSounds();
	
	/** 모든 활성 사운드 중지 */
	void StopAllActiveSounds();
	
	/** 새 사운드 ID 등록 (개선된 정리 시스템 포함) */
	void RegisterPlayingID(AkPlayingID NewPlayingID);
	
	/** EndOfEvent 콜백과 함께 사운드 재생 */
	AkPlayingID PostEventWithCallback(UAkAudioEvent* AkEvent, AActor* Actor);

	// =================
	// 거리 및 RTPC 관리
	// =================
	
	/** 거리 및 상태 체크 (타이머 콜백) */
	virtual void UpdateDistanceRTPC();
	
	/** RTPC 업데이트가 필요한지 확인 */
	bool ShouldUpdateRTPC(float NewDistance, float CurrentTime) const;
	
	/** 통일된 RTPC 값 설정 (0-1 → 0-100 자동 변환) */
	void SetUnifiedRTPCValue(UAkRtpc* RTPC, float NormalizedValue, float InterpolationTime = 0.0f);
	
	/** Distance Scaling 설정 (통일된 방식) */
	void SetDistanceScaling(bool bIsRTS);
	
	/** 모든 오디오 RTPC 초기화 */
	virtual void InitializeAudioRTPCs();

	// ===================
	// 가상 함수 (하위 클래스에서 구현)
	// ===================
	
	/** 거리 기반 상태 변경 체크 */
	virtual void CheckForStateChanges() {}
	
	/** 최대 오디오 거리 반환 */
	virtual float GetMaxAudioDistance() const { return TPSMaxDistance; }
	
	/** 특정 사운드 완료 시 추가 정리 작업 */
	virtual void OnSpecificSoundFinished(AkPlayingID FinishedID) {}

	/** Owner Actor의 AkComponent를 찾거나 생성하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	UAkComponent* GetOrCreateAkComponent();

	// Replication 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY()
	TObjectPtr<UAkComponent> CachedAkComponent; // Wwise Component Cache
};