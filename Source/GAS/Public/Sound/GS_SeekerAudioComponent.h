#pragma once

#include "CoreMinimal.h"
#include "Sound/GS_AudioComponentBase.h"
#include "Character/Skill/ESkill.h"
#include "Engine/TimerHandle.h"
#include "Net/UnrealNetwork.h"
#include "GS_SeekerAudioComponent.generated.h"

class AGS_Seeker;
class UAkAudioEvent;

/**
 * 시커의 오디오 상태 열거형
 * 가디언이 시커를 볼 때 들리는 사운드를 위한 시스템
 */
UENUM(BlueprintType)
enum class ESeekerAudioState : uint8
{
    Idle        UMETA(DisplayName = "평상시"),
    Combat      UMETA(DisplayName = "전투"),
    Aiming      UMETA(DisplayName = "조준 중"), 
    Hurt        UMETA(DisplayName = "피해받음"),
    Death       UMETA(DisplayName = "죽음")
};

/**
 * 시커 오디오 설정 구조체
 */
USTRUCT(BlueprintType)
struct FSeekerAudioConfig
{
    GENERATED_BODY()

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

    // 거리 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float MaxAudioDistance = 1500.0f; // 이 거리 밖에서는 아예 사운드 이벤트 발생 안함

    FSeekerAudioConfig()
    {
        CombatSound = nullptr;
        HurtSound = nullptr;
        DeathSound = nullptr;
        RTS_CombatSound = nullptr;
        RTS_HurtSound = nullptr;
        RTS_DeathSound = nullptr;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_SeekerAudioComponent : public UGS_AudioComponentBase
{
    GENERATED_BODY()

public:
    UGS_SeekerAudioComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ===================
    // 사운드 설정 (에디터에서 설정)
    // ===================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Core Settings", meta = (DisplayName = "Audio Configuration"))
    FSeekerAudioConfig AudioConfig;

    // 사운드 재생 간격 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float CombatSoundInterval = 5.0f;

    // ===================
    // 조준 관련 사운드 (에디터에서 설정)
    // ===================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Aiming Events", meta = (DisplayName = "Bow Draw Sound (TPS)"))
    UAkAudioEvent* BowDrawSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Aiming Events", meta = (DisplayName = "Bow Release Sound (TPS)"))
    UAkAudioEvent* BowReleaseSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Aiming Events", meta = (DisplayName = "RTS Bow Draw Sound"))
    UAkAudioEvent* RTS_BowDrawSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Aiming Events", meta = (DisplayName = "RTS Bow Release Sound"))
    UAkAudioEvent* RTS_BowReleaseSound = nullptr;

    // ===================
    // 화살 관련 사운드 (에디터에서 설정)
    // ===================
    
    // 화살 발사 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Arrow Events", meta = (DisplayName = "Arrow Shot Sound (TPS)"))
    UAkAudioEvent* ArrowShotSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Arrow Events", meta = (DisplayName = "RTS Arrow Shot Sound"))
    UAkAudioEvent* RTS_ArrowShotSound = nullptr;

    // 화살 타입 변경 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Arrow Events", meta = (DisplayName = "Arrow Type Change Sound"))
    UAkAudioEvent* ArrowTypeChangeSound = nullptr;

    // 화살 부족 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Arrow Events", meta = (DisplayName = "Arrow Empty Sound"))
    UAkAudioEvent* ArrowEmptySound = nullptr;

    // 타격 피드백 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Feedback Events", meta = (DisplayName = "Hit Feedback Sound"))
    UAkAudioEvent* HitFeedbackSound_Archery = nullptr;

    // ===================
    // 스킬 관련 사운드 (기존 CharacterAudioComponent 기능 통합)
    // ===================
    
    // 기본 스킬 이벤트 (에디터에서 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Skill Events", meta = (DisplayName = "Default Skill Event"))
    UAkAudioEvent* SkillEvent;

public:
    // ===================
    // 시커 상태 관리
    // ===================
    
    /** 시커 상태 변경 시 호출 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio")
    void SetSeekerAudioState(ESeekerAudioState NewState);

    /** 즉시 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio")
    void PlaySound(ESeekerAudioState SoundType, bool bForcePlay = false);

    /** 피해받을 때 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio")
    void PlayHurtSound();

    /** 죽을 때 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio")
    void PlayDeathSound();

    /** 현재 오디오 상태 반환 */
    UFUNCTION(BlueprintPure, Category = "Seeker Audio")
    ESeekerAudioState GetCurrentAudioState() const { return CurrentAudioState; }

    // ===================
    // 조준 관련 함수
    // ===================
    
    /** 활 당기기 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Aiming")
    void PlayBowDrawSound();

    /** 활 발사 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Aiming")
    void PlayBowReleaseSound();

    // ===================
    // 화살 관련 함수
    // ===================
    
    /** 화살 발사 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Arrow")
    void PlayArrowShotSound();

    /** 화살 타입 변경 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Arrow")
    void PlayArrowTypeChangeSound();

    /** 화살 부족 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Arrow")
    void PlayArrowEmptySound();

    /** 타격 피드백 사운드 재생 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Feedback")
    void PlayHitFeedbackSound();

    // ===================
    // 스킬 관련 함수 (기존 CharacterAudioComponent에서 이전)
    // ===================
    
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void PlaySkill();
    
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void StopSkill();
    
    // AkComponent 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "Sound|Character")
    class UAkComponent* GetOrCreateAkComponent();

    // 위치 기반 사운드 재생
    UFUNCTION(BlueprintCallable, Category = "Sound|Character")
    void PlaySoundAtLocation(UAkAudioEvent* SoundEvent, const FVector& Location);

    // 스킬 사운드 재생
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void PlaySkillSoundFromDataTable(ESkillSlot SkillSlot, bool bIsSkillStart = true);

    // 스킬 루프 사운드 재생/정지 (궁극기용)
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void PlaySkillLoopSoundFromDataTable(ESkillSlot SkillSlot);

    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void StopSkillLoopSoundFromDataTable(ESkillSlot SkillSlot);

    // 스킬 충돌 사운드 재생 (궁극기용) - CollisionType: 0=벽, 1=몬스터, 2=가디언
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void PlaySkillCollisionSoundFromDataTable(ESkillSlot SkillSlot, uint8 CollisionType);

    // Event-Driven 오디오 시스템
    UFUNCTION(BlueprintCallable, Category = "Sound|EventDriven")
    void RequestSkillAudio(ESkillSlot SkillSlot, int32 AudioEventType, FVector Location = FVector::ZeroVector);

    // 스킬셋 데이터 기반 사운드 재생 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "Sound|Character")
    void PlaySkillSoundFromSkillInfo(bool bIsSkillStart, UAkAudioEvent* SkillStartSound, UAkAudioEvent* SkillEndSound);

    // 콤보 공격 사운드 재생 (근접 공격 시커)
    UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
    void PlayComboAttackSound(UAkAudioEvent* SwingSound, UAkAudioEvent* VoiceSound, UAkAudioEvent* StopEvent, float ResetTime);
    
    // 콤보 인덱스별 공격 사운드 재생 (개선된 버전)
    UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
    void PlayComboAttackSoundByIndex(int32 ComboIndex, const TArray<UAkAudioEvent*>& SwingSounds, const TArray<UAkAudioEvent*>& VoiceSounds, UAkAudioEvent* StopEvent, float ResetTime);

    // 콤보 인덱스별 공격 사운드 재생 (추가 사운드 포함)
    UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
    void PlayComboAttackSoundByIndexWithExtra(int32 ComboIndex, const TArray<UAkAudioEvent*>& SwingSounds, const TArray<UAkAudioEvent*>& VoiceSounds, const TArray<UAkAudioEvent*>& ExtraSounds, UAkAudioEvent* StopEvent, float ResetTime);
    
    // 콤보 마지막 타격 특별 사운드 (근접 공격 시커)
    UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
    void PlayFinalAttackSound(UAkAudioEvent* ExtraSound);

    // 단일 사운드 재생 (원거리 공격 시커)
    UFUNCTION(BlueprintCallable, Category = "Sound|Generic")
    void PlayGenericSound(UAkAudioEvent* SoundToPlay, bool bPlayOnLocalOnly = false);

    // 피격 사운드 재생
    UFUNCTION(BlueprintCallable, Category = "Sound|Hit")
    void PlayHitSound();

    // RTS 커맨드 사운드는 시커에서 사용하지 않음

    // Replication 설정
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // BaseClass override
    virtual void CheckForStateChanges() override;
    virtual float GetMaxAudioDistance() const override;

private:
    UPROPERTY()
    TObjectPtr<AGS_Seeker> OwnerSeeker;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentAudioState)
    ESeekerAudioState CurrentAudioState;

    UPROPERTY()
    ESeekerAudioState PreviousAudioState;

    FTimerHandle IdleSoundTimer;
    FTimerHandle CombatSoundTimer;
    
    // 마지막 사운드 재생 시간
    float LastSoundPlayTime;
    
    // 스킬 이벤트 ID
    int32 SkillEventID;

    // 서버에서 클라이언트로 사운드 동기화
    TMap<ESeekerAudioState, float> LocalLastSoundPlayTimes;

    UPROPERTY(Transient) 
    TMap<ESeekerAudioState, float> ServerLastBroadcastTime;

    // 마지막 히트 사운드 재생 시간 (Deprecated)
    float LastHitSoundTime = 0.0f;

    // 콤보 공격 사운드 중지 콜백
    void ResetAttackSoundSequence();

    // 콤보 사운드 중지 이벤트 (내부 사용)
    UPROPERTY()
    UAkAudioEvent* CurrentStopEvent;

    FTimerHandle AttackSoundResetTimerHandle;

    /** 자동 사운드 재생 (타이머 콜백) */
    void PlayIdleSound();
    void PlayCombatSound();

    /** 타이머 관리 */
    void StartSoundTimer();
    void StopSoundTimer();
    void UpdateSoundTimer();

    /** Wwise 이벤트 실제 재생 (Wwise가 거리 감쇠 자동 처리) */
    UAkAudioEvent* GetSoundEvent(ESeekerAudioState SoundType) const;

    UFUNCTION()
    void OnRep_CurrentAudioState();

    // 클라이언트 사운드 재생 트리거용 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_TriggerSound(ESeekerAudioState SoundTypeToTrigger, bool bIsImmediate);

    // 조준 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayBowDrawSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayBowReleaseSound();

    // Hurt/Death 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayHurtSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeathSound();

    // 화살 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayArrowShotSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayArrowTypeChangeSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayArrowEmptySound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayHitFeedbackSound();

    // DT_SkillSet에서 스킬 정보 조회
    const struct FSkillInfo* GetSkillInfoFromDataTable(ESkillSlot SkillSlot) const;

    UPROPERTY()
    class UAkComponent* CachedAkComponent;
};