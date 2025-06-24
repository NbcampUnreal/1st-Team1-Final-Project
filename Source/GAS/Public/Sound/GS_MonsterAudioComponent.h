#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AkGameplayStatics.h"
#include "AkComponent.h"
#include "Engine/TimerHandle.h"
#include "Net/UnrealNetwork.h"
#include "GS_MonsterAudioComponent.generated.h"

class AGS_Monster;
class AGS_Seeker;

/**
 * 몬스터의 거리별 사운드를 관리하는 컴포넌트
 * RTS와 TPS 모드 모두에서 작동하는 몬스터 울음소리 시스템
 */
UENUM(BlueprintType)
enum class EMonsterAudioState : uint8
{
    Idle        UMETA(DisplayName = "평상시"),
    Combat      UMETA(DisplayName = "전투"),
    Hurt        UMETA(DisplayName = "피해받음"),
    Death       UMETA(DisplayName = "죽음")
};

USTRUCT(BlueprintType)
struct FMonsterAudioConfig
{
    GENERATED_BODY()

    // 거리별 사운드 이벤트들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events")
    UAkAudioEvent* IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events")
    UAkAudioEvent* CombatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events")
    UAkAudioEvent* HurtSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events")
    UAkAudioEvent* DeathSound;

    // 게임 로직용 거리 설정 (Wwise Attenuation과 별개)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Logic", meta = (ClampMin = "0.0"))
    float AlertDistance = 800.0f; // 이 거리 안에 시커가 있으면 Combat 상태

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float MaxAudioDistance = 1000.0f; // 이 거리 밖에서는 아예 사운드 이벤트 발생 안함

    FMonsterAudioConfig()
    {
        IdleSound = nullptr;
        CombatSound = nullptr;
        HurtSound = nullptr;
        DeathSound = nullptr;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_MonsterAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGS_MonsterAudioComponent();

    // RTPC 이름 상수
    static const FName DistanceToPlayerRTPCName;
    static const FName MonsterVariantRTPCName;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ==========
    // 사운드 설정
    // ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio")
    FMonsterAudioConfig AudioConfig;

    // 사운드 재생 간격 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float IdleSoundInterval = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float CombatSoundInterval = 4.0f;

    // 몬스터 종류별 고유 사운드 인덱스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio")
    int32 MonsterSoundVariant = 0;

    // ========
    // 공용 함수
    // ========
    
    /** 몬스터 상태 변경 시 호출 */
    UFUNCTION(BlueprintCallable, Category = "Monster Audio")
    void SetMonsterAudioState(EMonsterAudioState NewState);

    /** 즉시 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Monster Audio")
    void PlaySound(EMonsterAudioState SoundType, bool bForcePlay = false);

    /** 피해받을 때 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Monster Audio")
    void PlayHurtSound();

    /** 죽을 때 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Monster Audio")
    void PlayDeathSound();

    /** 현재 오디오 상태 반환 */
    UFUNCTION(BlueprintPure, Category = "Monster Audio")
    EMonsterAudioState GetCurrentAudioState() const { return CurrentAudioState; }

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // ======================
    // 내부 변수
    // ======================
    
    UPROPERTY()
    TObjectPtr<AGS_Monster> OwnerMonster;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentAudioState)
    EMonsterAudioState CurrentAudioState;

    UPROPERTY()
    EMonsterAudioState PreviousAudioState;

    FTimerHandle IdleSoundTimer;
    FTimerHandle CombatSoundTimer;
    FTimerHandle DistanceCheckTimerHandle;
    
    // 마지막 사운드 재생 시간
    float LastSoundPlayTime;
    
    // 현재 재생 중인 사운드 ID
    AkPlayingID CurrentPlayingID;

    // ========
    // 내부 함수
    // ========
    
    /** 가장 가까운 시커 찾기 */
    AGS_Seeker* FindNearestSeeker() const;

    /** 시커와의 거리 계산 */
    float CalculateDistanceToNearestSeeker() const;

    /** 자동 사운드 재생 (타이머 콜백) */
    void PlayIdleSound();
    void PlayCombatSound();

    /** 타이머 관리 */
    void StartSoundTimer();
    void StopSoundTimer();
    void UpdateSoundTimer();

    /** 거리 및 상태 체크 (타이머 콜백) */
    void UpdateDistanceRTPC();

    /** Wwise 이벤트 실제 재생 (Wwise가 거리 감쇠 자동 처리) */
    UAkAudioEvent* GetSoundEvent(EMonsterAudioState SoundType) const;
    
    /** 몬스터 상태 변화 감지 */
    void CheckForStateChanges();
    
    /** 디버그 정보 표시 */
    void DrawDebugInfo() const;

    UFUNCTION()
    void OnRep_CurrentAudioState();

    // New RPC for triggering sound check on clients
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_TriggerSound(EMonsterAudioState SoundTypeToTrigger, bool bIsImmediate);

    // Client-side map to track the last play time for each sound type to manage cooldowns locally
    TMap<EMonsterAudioState, float> LocalLastSoundPlayTimes;

    // Server-side map to track the last time a timed sound broadcast was made
    // Not replicated, used by server authority only.
    UPROPERTY(Transient) 
    TMap<EMonsterAudioState, float> ServerLastBroadcastTime;

    // === 성능 최적화 관련 ===
    // 거리 체크 최적화 - 동적 인터벌 조정
    float DynamicDistanceCheckInterval;
    float LastPlayerDistance;
    
    // LOD 시스템 - 거리에 따른 업데이트 빈도 조정
    enum class EAudioLOD : uint8
    {
        High,    // 가까운 거리 - 높은 빈도
        Medium,  // 중간 거리 - 중간 빈도  
        Low,     // 먼 거리 - 낮은 빈도
        Disabled // 매우 먼 거리 - 비활성화
    };
    
    EAudioLOD CurrentAudioLOD;
    
    // 플레이어 위치 캐싱
    FVector CachedPlayerLocation;
    float LastPlayerLocationCheckTime;
    
    // 성능 관련 설정값
    static constexpr float CLOSE_DISTANCE_THRESHOLD = 500.0f;
    static constexpr float MEDIUM_DISTANCE_THRESHOLD = 1000.0f;
    static constexpr float FAR_DISTANCE_THRESHOLD = 1500.0f;
    
    static constexpr float HIGH_LOD_INTERVAL = 0.2f;   // 5FPS
    static constexpr float MEDIUM_LOD_INTERVAL = 0.5f; // 2FPS
    static constexpr float LOW_LOD_INTERVAL = 1.0f;    // 1FPS
    
    // 최적화 함수들
    void UpdateAudioLOD(float Distance);
    void OptimizeUpdateFrequency();
    bool ShouldUpdateAudio() const;
}; 