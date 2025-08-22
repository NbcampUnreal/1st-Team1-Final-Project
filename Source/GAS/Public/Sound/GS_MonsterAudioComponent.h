#pragma once

#include "CoreMinimal.h"
#include "Sound/GS_AudioComponentBase.h"
#include "AkGameplayStatics.h"
#include "AkComponent.h"
#include "Engine/TimerHandle.h"
#include "Net/UnrealNetwork.h"
#include "GS_MonsterAudioComponent.generated.h"

class AGS_Monster;
class AGS_Seeker;
class AGS_RTSController;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events|RTS", meta = (DisplayName = "RTS Combat Sound"))
    UAkAudioEvent* RTS_CombatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events|RTS", meta = (DisplayName = "RTS Hurt Sound"))
    UAkAudioEvent* RTS_HurtSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events|RTS", meta = (DisplayName = "RTS Death Sound"))
    UAkAudioEvent* RTS_DeathSound;

    // 게임 로직용 거리 설정 (Wwise Attenuation과 별개)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Logic", meta = (ClampMin = "0.0"))
    float AlertDistance = 800.0f; // 이 거리 안에 시커가 있으면 Combat 상태

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float MaxAudioDistance = 1500.0f; // 이 거리 밖에서는 아예 사운드 이벤트 발생 안함

    FMonsterAudioConfig()
    {
        IdleSound = nullptr;
        CombatSound = nullptr;
        HurtSound = nullptr;
        DeathSound = nullptr;
        RTS_CombatSound = nullptr;
        RTS_HurtSound = nullptr;
        RTS_DeathSound = nullptr;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_MonsterAudioComponent : public UGS_AudioComponentBase
{
    GENERATED_BODY()

public:
    UGS_MonsterAudioComponent();

    // 몬스터 전용 RTPC 이름 상수
    static const FName MonsterVariantRTPCName;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ==========
    // 사운드 설정 (에디터에서 설정)
    // ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio|Core Settings", meta = (DisplayName = "Audio Configuration"))
    FMonsterAudioConfig AudioConfig;

    // 사운드 재생 간격 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float IdleSoundInterval = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float CombatSoundInterval = 4.0f;

    // 몬스터 종류별 고유 사운드 인덱스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio")
    int32 MonsterSoundVariant = 0;

    // 스윙 사운드 (에디터에서 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio|Swing Events", meta = (DisplayName = "Swing Sound (TPS)"))
    UAkAudioEvent* SwingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio|Swing Events", meta = (DisplayName = "RTS Swing Sound"))
    UAkAudioEvent* RTS_SwingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Audio|Swing", meta=(ClampMin="0.0"))
    float SwingResetTime = 0.2f;

    // RTS 커맨드 사운드 (에디터에서 설정)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster Audio|RTS Command Events", meta = (DisplayName = "Selection Click Sound"))
    UAkAudioEvent* SelectionClickSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster Audio|RTS Command Events", meta = (DisplayName = "RTS Move Command Sound"))
    UAkAudioEvent* RTSMoveCommandSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster Audio|RTS Command Events", meta = (DisplayName = "RTS Attack Command Sound"))
    UAkAudioEvent* RTSAttackCommandSound = nullptr;

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

    // 스윙 사운드 재생
    UFUNCTION(BlueprintCallable, Category = "Monster Audio|Swing")
    void PlaySwingSound();

    // RTS 커맨드 사운드
    UFUNCTION(BlueprintCallable, Category = "Monster Audio|RTS")
    void PlayRTSCommandSound(ERTSCommandSoundType CommandType);

    // Replication 설정
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // BaseClass override
    virtual void CheckForStateChanges() override;
    virtual float GetMaxAudioDistance() const override;

private:
    UPROPERTY()
    TObjectPtr<AGS_Monster> OwnerMonster;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentAudioState)
    EMonsterAudioState CurrentAudioState;

    UPROPERTY()
    EMonsterAudioState PreviousAudioState;

    FTimerHandle IdleSoundTimer;
    FTimerHandle CombatSoundTimer;
    
    // 마지막 사운드 재생 시간
    float LastSoundPlayTime;
    
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

    /** Wwise 이벤트 실제 재생 (Wwise가 거리 감쇠 자동 처리) */
    UAkAudioEvent* GetSoundEvent(EMonsterAudioState SoundType) const;

    /** 디버그 정보 표시 */
    void DrawDebugInfo() const;

    UFUNCTION()
    void OnRep_CurrentAudioState();

    // 클라이언트 사운드 재생 트리거용 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_TriggerSound(EMonsterAudioState SoundTypeToTrigger, bool bIsImmediate);

    // 스윙 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySwingSound();

    TMap<EMonsterAudioState, float> LocalLastSoundPlayTimes;

    UPROPERTY(Transient) 
    TMap<EMonsterAudioState, float> ServerLastBroadcastTime;

    // 스윙 사운드 쿨다운 기록용 변수
    UPROPERTY(Transient)
    float ServerLastSwingBroadcastTime = -1000.0f;

    UPROPERTY(Transient)
    float LocalLastSwingPlayTime = -1000.0f;
}; 