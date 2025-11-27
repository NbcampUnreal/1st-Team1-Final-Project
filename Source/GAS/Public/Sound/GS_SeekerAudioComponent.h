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
    UAkAudioEvent* HurtSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events")
    UAkAudioEvent* DeathSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events|RTS", meta = (DisplayName = "RTS Hurt Sound"))
    UAkAudioEvent* RTS_HurtSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Events|RTS", meta = (DisplayName = "RTS Death Sound"))
    UAkAudioEvent* RTS_DeathSound;

    // 거리 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float MaxAudioDistance = 1500.0f; // 이 거리 밖에서는 아예 사운드 이벤트 발생 안함

    FSeekerAudioConfig()
    {
        HurtSound = nullptr;
        DeathSound = nullptr;
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
    virtual void OnSpecificSoundFinished(AkPlayingID FinishedID) override;

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

    // ===================
    // TPS Sounds (Third Person Shooter 모드)
    // ===================
    
    // 찬 전용 TPS 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🛡️ Shield Slam Start Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ShieldSlamStartSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🛡️ Shield Slam Impact Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ShieldSlamImpactSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🪓 Axe Swing Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanAxeSwingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🪓 Axe Swing Stop Event", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanAxeSwingStopEvent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🪓 Final Attack Extra Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanFinalAttackExtraSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🗣️ Attack Voice Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanAttackVoiceSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Chan", meta = (DisplayName = "🛡️ Defense Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* ChanDefenseSound = nullptr;

    // 아레스 전용 TPS 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Ares", meta = (DisplayName = "⚔️ Sword Swing Stop Event", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    UAkAudioEvent* AresSwordSwingStopEvent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Ares", meta = (DisplayName = "⚔️ Combo Swing Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> AresComboSwingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Ares", meta = (DisplayName = "🗣️ Combo Voice Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> AresComboVoiceSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Ares", meta = (DisplayName = "✨ Combo Extra Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> AresComboExtraSounds;

    // 메르시 전용 TPS 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Merci", meta = (DisplayName = "🏹 Bow Draw Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* BowDrawSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Merci", meta = (DisplayName = "🏹 Bow Release Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* BowReleaseSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Merci", meta = (DisplayName = "🏹 Arrow Shot Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* ArrowShotSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Merci", meta = (DisplayName = "🏹 Arrow Type Change Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* ArrowTypeChangeSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Merci", meta = (DisplayName = "🏹 Arrow Empty Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* ArrowEmptySound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|TPS Sounds|Merci", meta = (DisplayName = "🏹 Hit Feedback Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* HitFeedbackSound = nullptr;

    // ===================
    // RTS Sounds (Real Time Strategy 모드)
    // ===================
    
    // 찬 전용 RTS 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Chan", meta = (DisplayName = "🛡️ RTS Shield Slam Start Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSShieldSlamStartSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Chan", meta = (DisplayName = "🛡️ RTS Shield Slam Impact Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSShieldSlamImpactSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Chan", meta = (DisplayName = "🪓 RTS Axe Swing Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSChanAxeSwingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Chan", meta = (DisplayName = "🗣️ RTS Attack Voice Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSChanAttackVoiceSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Chan", meta = (DisplayName = "🛡️ RTS Defense Sound", EditCondition = "CharacterType == ECharacterType::Chan", EditConditionHides))
    UAkAudioEvent* RTSChanDefenseSound = nullptr;

    // 아레스 전용 RTS 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Ares", meta = (DisplayName = "⚔️ RTS Sword Swing Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> RTSAresSwordSwingSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Ares", meta = (DisplayName = "🗣️ RTS Combo Voice Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> RTSAresComboVoiceSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Ares", meta = (DisplayName = "✨ RTS Combo Extra Sounds Array", EditCondition = "CharacterType == ECharacterType::Ares", EditConditionHides))
    TArray<UAkAudioEvent*> RTSAresComboExtraSounds;

    // 메르시 전용 RTS 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Merci", meta = (DisplayName = "🏹 RTS Bow Draw Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* RTSMerciBowDrawSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Merci", meta = (DisplayName = "🏹 RTS Bow Release Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* RTSMerciBowReleaseSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|RTS Sounds|Merci", meta = (DisplayName = "🏹 RTS Arrow Shot Sound", EditCondition = "CharacterType == ECharacterType::Merci", EditConditionHides))
    UAkAudioEvent* RTSMerciArrowShotSound = nullptr;

    // ===================
    // Common Sounds (공통 사운드)
    // ===================

    // 스킬 관련 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|Common Sounds", meta = (DisplayName = "Default Skill Event"))
    UAkAudioEvent* SkillEvent = nullptr;

    // ===================
    // LowHP Pain Sound System (LowHP 통증 사운드 시스템)
    // ===================

    /** LowHP 루핑 사운드 (HP 30% 이하 시 재생) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|LowHP Pain", meta = (DisplayName = "🩸 LowHP Pain Loop Sound"))
    UAkAudioEvent* LowHPPainLoopSound = nullptr;

    /** LowHP 중지 이벤트 (페이드아웃 포함) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|LowHP Pain", meta = (DisplayName = "🩸 LowHP Pain Stop Event"))
    UAkAudioEvent* LowHPPainStopSound = nullptr;

    /** LowHP 볼륨 제어 RTPC */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|LowHP Pain", meta = (DisplayName = "🩸 LowHP Pain Volume RTPC"))
    UAkRtpc* LowHPPainVolumeRTPC = nullptr;

    /** LowHP 필터 제어 RTPC */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|LowHP Pain", meta = (DisplayName = "🩸 LowHP Pain Filter RTPC"))
    UAkRtpc* LowHPPainFilterRTPC = nullptr;

    /** LowHP 임계값 (기본 30%) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|LowHP Pain", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "🩸 LowHP Threshold (0.0~1.0)"))
    float LowHPThreshold = 0.3f;

    // ===================
    // UI Sounds
    // ===================
    
    /** 가디언 감지 경고음 (UI Sound) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|UI Sounds", meta = (DisplayName = "🔔 Detection Warning Sound"))
    USoundBase* DetectionWarningSound = nullptr;
    
    /** 가디언 감지 해제 안도음 (UI Sound) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seeker Audio|UI Sounds", meta = (DisplayName = "✅ Detection Cleared Sound"))
    USoundBase* DetectionClearedSound = nullptr;

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

    /** 피해받을 때 사운드 (로컬 전용 - RPC 없음) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio")
    void PlayHurtSoundLocal();

    /** 죽을 때 사운드 (로컬 전용 - RPC 없음) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio")
    void PlayDeathSoundLocal();

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
    // UI 사운드 함수 (가디언 감지 시스템)
    // ===================
    
    /** 가디언 감지 경고음 재생 (UI Sound) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|UI Sounds")
    void PlayDetectionWarningSound();
    
    /** 가디언 감지 해제 안도음 재생 (UI Sound) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|UI Sounds")
    void PlayDetectionClearedSound();

    // ===================
    // LowHP Pain Sound 제어 함수
    // ===================

    /** LowHP 통증 사운드 시작 (로컬 전용 - RPC 없음) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|LowHP Pain")
    void StartLowHPPainSound();

    /** LowHP 통증 사운드 중지 (로컬 전용 - RPC 없음) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|LowHP Pain")
    void StopLowHPPainSound();

    /** LowHP 통증 사운드 볼륨/필터 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|LowHP Pain")
    void UpdateLowHPPainVolume(float CurrentHP, float MaxHP);

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

    /** 방어 사운드 (찬 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Chan Only", meta = (CallInEditor = "true"))
    void PlayDefenseSound();

    // ===================
    // 아레스 전용 함수 (Ares Only Functions)
    // ===================
    
    /** 콤보 공격 사운드 (아레스 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ares Only", meta = (CallInEditor = "true"))
    void PlayAresComboAttackSound(int32 ComboIndex);

    /** 콤보 공격 사운드 (추가 사운드 포함, 아레스 전용) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|Ares Only", meta = (CallInEditor = "true"))
    void PlayAresComboAttackSoundWithExtra(int32 ComboIndex);
    
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

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_RequestSkillAudio(ESkillSlot SkillSlot, int32 AudioEventType, FVector Location = FVector::ZeroVector);

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

    // ===================
    // TPS 콤보 사운드 멀티캐스트 RPC
    // ===================
    
    // 찬 전용 TPS 콤보 공격 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayChanComboAttackSound(int32 ComboIndex);

    // 아레스 전용 TPS 콤보 공격 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAresComboAttackSound(int32 ComboIndex);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAresComboAttackSoundWithExtra(int32 ComboIndex);

    // 찬 전용 방어 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDefenseSound();

    // ===================
    // RTS 콤보 사운드 통합 함수들
    // ===================
    
    /** RTS 모드에서 찬의 공격 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Combo")
    void PlayRTSChanAttackSound();

    /** RTS 모드에서 찬의 방패 슬램 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Combo")
    void PlayRTSChanShieldSlamSound();

    /** RTS 모드에서 아레스의 콤보 공격 사운드 (배열 기반) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Combo")
    void PlayRTSAresComboAttackSound(int32 ComboIndex);

    /** RTS 모드에서 아레스의 콤보 공격 사운드 (추가 사운드 포함, 배열 기반) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Combo")
    void PlayRTSAresComboAttackSoundWithExtra(int32 ComboIndex);

    // 단일 사운드 재생 (원거리 공격 시커)
    UFUNCTION(BlueprintCallable, Category = "Sound|Generic")
    void PlayGenericSound(UAkAudioEvent* SoundToPlay, bool bPlayOnLocalOnly = false);

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

    // 타이머 핸들
    UPROPERTY()
    FTimerHandle CombatSoundTimerHandle;
    
    // 스킬 이벤트 ID (선택적 정지를 위해)
    AkPlayingID SkillEventID;

    // 서버에서 클라이언트로 사운드 동기화
    TMap<ESeekerAudioState, float> LocalLastSoundPlayTimes;

    UPROPERTY(Transient) 
    TMap<ESeekerAudioState, float> ServerLastBroadcastTime;

    // 콤보 공격 사운드 중지 콜백
    void ResetAttackSoundSequence();

    // 콤보 사운드 중지 이벤트 (내부 사용)
    UPROPERTY()
    UAkAudioEvent* CurrentStopEvent;

    UPROPERTY()
    FTimerHandle AttackSoundResetTimerHandle;

    // 컴포넌트 셧다운 상태 플래그 (레벨 전환/액터 파괴 시 RPC 크래시 방지)
    bool bIsComponentShuttingDown;

    // ===================
    // LowHP Pain Sound 내부 상태 변수
    // ===================

    /** LowHP Pain 사운드 재생 중 여부 */
    bool bIsLowHPPainPlaying = false;

    /** LowHP Pain Playing ID (중지 시 사용) */
    AkPlayingID LowHPPainPlayingID = AK_INVALID_PLAYING_ID;

    /** LowHP 체크 타이머 핸들 (0.5초마다 HP 체크) */
    FTimerHandle LowHPCheckTimerHandle;

    /** 마지막 볼륨 비율 (중복 RTPC 호출 방지) */
    float LastLowHPVolumeRatio = -1.0f;

    /** 마지막 필터 비율 (중복 RTPC 호출 방지) */
    float LastLowHPFilterRatio = -1.0f;

    /** 타이머 관리 */
    void StartSoundTimer();
    void StopSoundTimer();
    void UpdateSoundTimer();

    // ===================
    // LowHP Pain Sound 타이머 콜백 및 헬퍼
    // ===================

    /** LowHP 체크 타이머 콜백 (0.5초마다 호출) */
    UFUNCTION()
    void OnLowHPPainCheck();

    /** LowHP Pain 사운드 강제 중지 (EndPlay() 호출용) */
    void ForceStopLowHPPainSound();

    /** Wwise 이벤트 실제 재생 (Wwise가 거리 감쇠 자동 처리) */
    UAkAudioEvent* GetSoundEvent(ESeekerAudioState SoundType) const;

    // ===================
    // RTS 사운드 재생 함수들
    // ===================
    
    /** RTS 모드에서 아레스의 검 휘두르기 사운드 (콤보 인덱스 기반) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Sounds")
    void PlayRTSAresSwordSwingSound(int32 ComboIndex = 0);

    /** RTS 모드에서 아레스의 콤보 보이스 사운드 (콤보 인덱스 기반) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Sounds")
    void PlayRTSAresComboVoiceSound(int32 ComboIndex = 0);

    /** RTS 모드에서 아레스의 콤보 추가 사운드 (콤보 인덱스 기반) */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Sounds")
    void PlayRTSAresComboExtraSound(int32 ComboIndex = 0);

    /** RTS 모드에서 메르시의 활 당기기 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Sounds")
    void PlayRTSMerciBowDrawSound();

    /** RTS 모드에서 메르시의 화살 발사 사운드 */
    UFUNCTION(BlueprintCallable, Category = "Seeker Audio|RTS Sounds")
    void PlayRTSMerciArrowShotSound();

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

    // 방패 슬램 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayShieldSlamStartSound();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayShieldSlamImpactSound();

    // 화살 사운드 멀티캐스트 RPC
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayArrowShotSound();

    // ===================
    // 공통 헬퍼 함수들
    // ===================
    
    /** 공통 거리 및 시야각 체크 함수 */
    UFUNCTION(BlueprintPure, Category = "Seeker Audio|Helpers")
    bool ShouldPlaySoundAtLocation(const FVector& SourceLocation, bool bSkipViewFrustumCheck = false) const;
    
    /** 콤보 인덱스 유효성 검사 및 변환 (1-based -> 0-based) */
    UFUNCTION(BlueprintPure, Category = "Seeker Audio|Helpers")
    int32 ValidateAndConvertComboIndex(int32 ComboIndex, int32 ArraySize) const;
    
    // DT_SkillSet에서 스킬 정보 조회
    const struct FSkillInfo* GetSkillInfoFromDataTable(ESkillSlot SkillSlot) const;

    /** 콤보 공격 사운드 재생 공통 로직 */
    void PlayComboSounds(int32 ArrayIndex, const TArray<UAkAudioEvent*>& SwingSounds, 
                         const TArray<UAkAudioEvent*>& VoiceSounds, 
                         const TArray<UAkAudioEvent*>* ExtraSounds = nullptr,
                         UAkAudioEvent* StopEvent = nullptr, float ResetTime = 0.0f);

    // ===================
    // 상수 정의
    // ===================
    
    // 콤보 인덱스 변환 상수
    static constexpr int32 ComboIndexOffset = 1; // 1-based를 0-based로 변환하기 위한 오프셋
};