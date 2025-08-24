#pragma once

#include "CoreMinimal.h"
#include "Sound/GS_AudioComponentBase.h"
#include "Character/Skill/ESkill.h"
#include "Engine/TimerHandle.h"
#include "Net/UnrealNetwork.h"
#include "Character/GS_Character.h"
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
    // 캐릭터 타입 설정
    // ===================
    // 캐릭터 타입 (GS_Character의 ECharacterType과 연동)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Character Type", meta = (DisplayName = "Character Type"))
    ECharacterType CharacterType;

    // 캐릭터 타입 getter
    UFUNCTION(BlueprintPure, Category = "Seeker Audio|Character Type")
    ECharacterType GetCharacterType() const { return CharacterType; }

    // 캐릭터 타입별 체크 함수들
    UFUNCTION(BlueprintPure, Category = "Seeker Audio|Character Type")
    bool IsChan() const { return CharacterType == ECharacterType::Chan; }
    
    UFUNCTION(BlueprintPure, Category = "Seeker Audio|Character Type")
    bool IsAres() const { return CharacterType == ECharacterType::Ares; }
    
    UFUNCTION(BlueprintPure, Category = "Seeker Audio|Character Type")
    bool IsMerci() const { return CharacterType == ECharacterType::Merci; }

    // ===================
    // 사운드 설정 (에디터에서 설정)
    // ===================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Core Settings", meta = (DisplayName = "Audio Configuration"))
    FSeekerAudioConfig AudioConfig;

    // 사운드 재생 간격 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float CombatSoundInterval = 1.0f;

    // ===================
    // 메르시 전용 사운드 (Merci Only - ECharacterType::Merci = 2)
    // ===================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Bow Draw Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* BowDrawSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 RTS Bow Draw Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* RTSBowDrawSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Bow Release Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* BowReleaseSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 RTS Bow Release Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* RTSBowReleaseSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Arrow Shot Sound (TPS)", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* ArrowShotSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Arrow Shot Sound (RTS)", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* RTSArrowShotSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Arrow Type Change Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* ArrowTypeChangeSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Arrow Empty Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* ArrowEmptySound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Merci Only", meta = (DisplayName = "🏹 Hit Feedback Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* HitFeedbackSound = nullptr;

    // ===================
    // 스킬 관련 사운드
    // ===================
    
    // 기본 스킬 이벤트
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Skill Events", meta = (DisplayName = "Default Skill Event"))
    UAkAudioEvent* SkillEvent = nullptr;

    // ===================
    // 찬 전용 사운드 (Chan Only - ECharacterType::Chan = 1) 
    // ===================
    
    // 방패 슬램 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🛡️ Shield Slam Start Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ShieldSlamStartSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🛡️ Shield Slam Impact Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ShieldSlamImpactSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🛡️ RTS Shield Slam Start Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSShieldSlamStartSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🛡️ RTS Shield Slam Impact Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSShieldSlamImpactSound = nullptr;

    // 찬 콤보 공격 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🪓 Axe Swing Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanAxeSwingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🪓 Axe Swing Stop Event", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanAxeSwingStopEvent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🪓 Final Attack Extra Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanFinalAttackExtraSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Chan Only", meta = (DisplayName = "🗣️ Attack Voice Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanAttackVoiceSound = nullptr;

    // ===================
    // 아레스 전용 사운드 (Ares Only - ECharacterType::Ares = 0)
    // ===================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Ares Only", meta = (DisplayName = "⚔️ Sword Swing Stop Event", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    UAkAudioEvent* AresSwordSwingStopEvent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Ares Only", meta = (DisplayName = "⚔️ Combo Swing Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> AresComboSwingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Ares Only", meta = (DisplayName = "🗣️ Combo Voice Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> AresComboVoiceSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Ares Only", meta = (DisplayName = "✨ Combo Extra Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> AresComboExtraSounds;

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

    /** 원거리 캐릭터인지 확인 */
    UFUNCTION(BlueprintPure, Category = "Seeker Audio")
    bool IsRangedCharacter() const { return CharacterType == ECharacterType::Merci; }

    // ===================
    // 원거리 캐릭터 전용 함수 (메르시 전용)
    // ===================
    
    /** 활 당기기 사운드 재생 (원거리 캐릭터 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ranged Only (Merci)")
    void PlayBowDrawSound();

    /** 활 발사 사운드 재생 (원거리 캐릭터 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ranged Only (Merci)")
    void PlayBowReleaseSound();

    /** 화살 발사 사운드 재생 (원거리 캐릭터 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ranged Only (Merci)")
    void PlayArrowShotSound();

    /** 화살 타입 변경 사운드 재생 (원거리 캐릭터 전용) */
    void PlayArrowTypeChangeSound();

    /** 화살 부족 사운드 재생 (메르시 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Merci Only", meta = (CallInEditor = "true"))
    void PlayArrowEmptySound();

    /** 타격 피드백 사운드 재생 (메르시 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Merci Only", meta = (CallInEditor = "true"))
    void PlayHitFeedbackSound();

    // ===================
    // 스킬 관련 함수
    // ===================
    
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void PlaySkill();
    
    UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
    void StopSkill();

    // ===================
    // 찬 전용 함수 (Chan Only Functions)
    // ===================
    
    /** 방패 슬램 시작 사운드 (찬 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Chan Only", meta = (CallInEditor = "true"))
    void PlayShieldSlamStartSound();

    /** 방패 슬램 충돌 사운드 (찬 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Chan Only", meta = (CallInEditor = "true"))
    void PlayShieldSlamImpactSound();

    /** 콤보 공격 사운드 (찬 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Chan Only", meta = (CallInEditor = "true"))
    void PlayChanComboAttackSound(int32 ComboIndex);

    /** 최종 공격 사운드 (찬 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Chan Only", meta = (CallInEditor = "true"))
    void PlayChanFinalAttackSound();

    // ===================
    // 아레스 전용 함수 (Ares Only Functions)
    // ===================
    
    /** 콤보 공격 사운드 (아레스 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ares Only", meta = (CallInEditor = "true"))
    void PlayAresComboAttackSound(int32 ComboIndex);

    /** 콤보 공격 사운드 (추가 사운드 포함, 아레스 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ares Only", meta = (CallInEditor = "true"))
    void PlayAresComboAttackSoundWithExtra(int32 ComboIndex);
    
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
    // Owner 참조 (하위 호환성)
    UPROPERTY()
    TObjectPtr<AGS_Seeker> OwnerSeeker;

    // Owner 참조 (GS_Character 기반)
    UPROPERTY()
    TObjectPtr<AGS_Character> OwnerCharacter;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentAudioState)
    ESeekerAudioState CurrentAudioState;

    UPROPERTY()
    ESeekerAudioState PreviousAudioState;

    FTimerHandle IdleSoundTimer;
    // 타이머 핸들
    UPROPERTY()
    FTimerHandle CombatSoundTimerHandle;

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

    UPROPERTY()
    FTimerHandle AttackSoundResetTimerHandle;

    /** 자동 사운드 재생 (타이머 콜백) */
    void PlayIdleSound();
    void PlayCombatSound();

    /** 타이머 관리 */
    void StartSoundTimer();
    void StopSoundTimer();
    void UpdateSoundTimer();
    
    /** 캐릭터 타입별 설정 검증 */
    void ValidateCharacterTypeSetup();

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

    // 방패 슬램 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayShieldSlamStartSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayShieldSlamImpactSound();

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