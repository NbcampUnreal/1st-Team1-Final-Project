// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "Character/Interface/GS_ManualDataInterface.h"
#include "NiagaraComponent.h"
#include "Animation/Character/E_SeekerAnim.h"
#include "Props/Item/E_ItemType.h"
#include "Character/Skill/GS_SkillComp.h"

#include "GS_Seeker.generated.h"

class UGS_SkillInputHandlerComp;
class UPostProcessComponent;
class UMaterialInterface;
class UGS_StatComp;
class AGS_PlayerState;
class UGS_VFXComponent;
class AGS_Monster;
class UGS_SeekerAudioComponent;
class UUserWidget;
class UGS_LowHealthEffectComponent;
class UGS_DetectionEffectComponent;
class AGS_Item;
class UGS_MarkerPlacementComponent;

USTRUCT(BlueprintType) // Current Action
struct FSeekerState
{
	GENERATED_BODY()

	FSeekerState()
	{
		IsAim = false;
		IsDraw = false;
		IsEquip = false;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsAim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsDraw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsEquip;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeekerHover, bool, bIsHover);

// 빈사 상태 변화 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDyingStateChanged, bool, bIsDying, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReviveProgressChanged, float, Progress);

// 충돌 사운드 타입 열거형
UENUM(BlueprintType)
enum class ECollisionSoundType : uint8
{
	Wall,
	Monster,
	Guardian
};

UCLASS()
class GAS_API AGS_Seeker : public AGS_Player, public IGS_ManualDataInterface
{
	GENERATED_BODY()

public:
	AGS_Seeker();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Death
	virtual void OnDeath() override;

	// State
	UFUNCTION(BlueprintCallable)
	void SetAimState(bool IsAim);

	UFUNCTION(BlueprintPure, Category = "State")
	bool GetAimState();

	UFUNCTION(BlueprintCallable)
	void SetDrawState(bool IsDraw);

	UFUNCTION(BlueprintPure, Category = "State")
	bool GetDrawState();

	UFUNCTION(Server, Reliable, Category = "State")
	void Server_SetSeekerGait(EGait Gait);

	UFUNCTION()
	void SetSeekerGait(EGait Gait);

	UFUNCTION(BlueprintCallable, Category = "State")
	EGait GetSeekerGait();

	UFUNCTION(BlueprintCallable, Category = "State")
	EGait GetLastSeekerGait();

	UFUNCTION()
	void StateReset();

	UFUNCTION()
	void OnRep_SeekerGait();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMontageSlot(ESeekerMontageSlot InputMontageSlot);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMustTurnInPlace(bool MustTurn);

	// Combo
	UFUNCTION(Server, Reliable)
	void Server_SetNextComboFlag(bool NextCombo);

	UFUNCTION(Server, Reliable)
	void Server_SetComboInputFlag(bool InputCombo);

	UFUNCTION(Server, Reliable)
	virtual void ServerAttackMontage();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastPlayComboSection();

	UFUNCTION()
	void ComboInputOpen();

	UFUNCTION()
	void ComboInputClose();

	UFUNCTION(Server, Reliable)
	virtual void Server_OnComboAttack();

	// Control
	UFUNCTION()
	void SetMoveControlValue(bool bMoveForward, bool bMoveRight);
	UFUNCTION()
	void SetLookControlValue(bool bLookUp, bool bLookRight);

	// Replication Set
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// === Audio Functions ===
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound(class UAkAudioEvent* SoundToPlay);

	// ===============
	// 공격 사운드 리셋 관련
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float AttackSoundResetTime = 1.0f;

	FTimerHandle AttackSoundResetTimerHandle;

	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	UChildActorComponent* Weapon;

	// Item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TMap<EItemType, UGS_ItemData*> ItemDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TMap<EItemType, AGS_Item*> Items;

	/*UFUNCTION()
	void ItemInit();



	UFUNCTION()
	void SetItem(EItemType ItemType);*/

	UFUNCTION()
	UGS_ItemData* GetItemData(EItemType ItemType);

	UFUNCTION()
	AGS_Item* GetItem(EItemType ItemType);

	// State
	UPROPERTY(Replicated)
	bool CanChangeSeekerGait;

	// Combo
	/*UPROPERTY(Replicated)
	bool bComboEnded = true;*/

	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* ComboAnimMontage;

	UPROPERTY(Replicated)
	int32 CurrentComboIndex;

	UPROPERTY(Replicated)
	bool CanAcceptComboInput = true;

	UPROPERTY(Replicated)
	bool bNextCombo = false;

	UPROPERTY(ReplicatedUsing = OnRep_SeekerGait)
	EGait SeekerGait;

	UPROPERTY(Replicated)
	EGait LastSeekerGait;

	UPROPERTY(BlueprintAssignable, Category="RTS")
	FOnSeekerHover OnSeekerHover;

	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
	UPostProcessComponent* LowHealthPostProcessComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects")
	UMaterialInterface* LowHealthEffectMaterial;

	// LowHealth 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
	UGS_LowHealthEffectComponent* LowHealthEffectComp;

	// ================
	// 가디언 감지 스크린 효과
	// ================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Detection|Effects")
	UPostProcessComponent* DetectionPostProcessComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Detection|Effects")
	UMaterialInterface* DetectionEffectMaterial; // MPP_Detect

	// Detection 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Detection|Effects")
	UGS_DetectionEffectComponent* DetectionEffectComp;

	UFUNCTION()
	void HandleLowHealthEffect(UGS_StatComp* InStatComp);

	// =======================
	// VFX 컴포넌트 (디버프, 힐링 등 모든 VFX)
	// =======================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UGS_VFXComponent* VFXComponent;

	// =======================
	// 시커 오디오 컴포넌트 (RTS/TPS 지원)
	// =======================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UGS_SeekerAudioComponent* SeekerAudioComponent;

	// =======================
	// 마커 배치 컴포넌트
	// =======================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker")
	UGS_MarkerPlacementComponent* MarkerPlacementComponent;

	// ================
	// 함정 VFX 컴포넌트
	// ================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	UNiagaraComponent* FeetLavaVFX_L;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	UNiagaraComponent* FeetLavaVFX_R;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX")
	UNiagaraComponent* BodyLavaVFX;

	// ================
	// 전투 음악 관리
	// ================
	// 몬스터 감지용 컴포넌트 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* CombatTrigger;

	// 전투 탐지 반경
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta=(ClampMin="0"))
	float CombatTriggerRadius = 800.0f;

	// 몬스터가 전투 음악 시작/중지를 요청할 때 호출
	UFUNCTION(BlueprintCallable)
	void AddCombatMonster(AGS_Monster* Monster);

	UFUNCTION(BlueprintCallable)
	void RemoveCombatMonster(AGS_Monster* Monster);

	// 새로운 몬스터 감지 시스템 (시커의 CombatTrigger)
	UFUNCTION()
	void OnCombatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCombatTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 상수들
	static const FName HPRatioParamName;
	static const FName EffectIntensityParamName;

	// Post Process 설정
	void InitializeCameraManager();
	void UpdatePostProcessEffect(float EffectStrength);

	// KeyManual을 위한 인터페이스 함수
	virtual FName GetManualRowName_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	UGS_SkillInputHandlerComp* SkillInputHandlerComponent;

	// 동적 머티리얼 파라미터 사용
	UPROPERTY()
	UMaterialInstanceDynamic* LowHealthDynamicMaterial;

	// 감지 효과용 동적 머티리얼
	UPROPERTY()
	UMaterialInstanceDynamic* DetectionDynamicMaterial;

	// 카메라 매니저 참조 추가
	UPROPERTY()
	APlayerCameraManager* LocalCameraManager;

	// KeyManual을 위한 캐릭터 타입 저장
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Manual")
	FName ManualRowName;

	// ===================================
	// LowHP 스크린 효과 (효과 보간 관련 변수)
	// ===================================
	UPROPERTY()
	float TargetEffectStrength;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectInterpSpeed = 2.0f; // 효과 보간 속도

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectFadeInSpeed = 1.0f; // 효과 페이드 인 속도

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float EffectFadeOutSpeed = 0.5f; // 효과 페이드 아웃 속도

	UPROPERTY(ReplicatedUsing = OnRep_IsLowHealthEffectActive)
	bool bIsLowHealthEffectActive;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentEffectStrength)
	float CurrentEffectStrength;

	UFUNCTION()
	void OnRep_IsLowHealthEffectActive();

	UFUNCTION()
	void OnRep_CurrentEffectStrength();

	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(EditDefaultsOnly, Category="Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LowHealthThresholdRatio = 0.3f;

	virtual void OnHoverBegin() override;
	virtual void OnHoverEnd() override;
	virtual FLinearColor GetCurrentDecalColor() override;
	virtual bool ShowDecal() override;

private:
	UPROPERTY(VisibleAnywhere, Category="State", Replicated)
	FSeekerState SeekerState;

	UPROPERTY()
	TArray<AGS_Monster*> NearbyMonsters;

	UPROPERTY()
	FTimerHandle LowHealthEffectTimer;

	// 가디언 감지 상태
	UPROPERTY(ReplicatedUsing = OnRep_IsDetectedByGuardian)
	bool bIsDetectedByGuardian = false;

	UFUNCTION()
	void OnRep_IsDetectedByGuardian();

	// 감지 사운드 쿨다운 (마지막 재생 시간 추적)
	UPROPERTY()
	float LastDetectionSoundTime = 0.0f;

	// 감지 사운드 최소 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Detection|Audio", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float DetectionSoundCooldown = 7.0f;

	// 퇴장 감지 사운드 쿨다운 (마지막 재생 시간 추적)
	UPROPERTY()
	float LastExitDetectionSoundTime = 0.0f;

	// 퇴장 감지 사운드 최소 간격 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Detection|Audio", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float ExitDetectionSoundCooldown = 7.0f;

	// 화면 중앙 근접도 (0.0 = 가장자리, 1.0 = 중앙)
	UPROPERTY(ReplicatedUsing = OnRep_DetectionIntensity)
	float DetectionIntensity = 0.0f;

	UFUNCTION()
	void OnRep_DetectionIntensity();

	// ==========================================
	// 가디언 감지 HUD 시스템
	// ==========================================

	/** 감지 HUD 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|Detection")
	TSubclassOf<class UUserWidget> DetectionHUDWidgetClass;

	void StartCombatMusic();
	void StopCombatMusic();

	UFUNCTION(Client, Unreliable)
	void ClientRPCStopCombatMusic();

	void UpdateCombatMusicState();
	void UpdateLowHealthEffect();

	// 플레이어 상태 변경 처리
	void HandleAliveStatusChanged(AGS_PlayerState* ChangedPlayerState, bool bIsNowAlive);

public:
	// RequiredCurState 가 현재 캐릭터의 상태와 같다면 캐릭터의 상태를 NextState 로 변경하고 TargetAM 을 재생한다.
	void TransWeaponHandingState(EWeaponHandlingState RequiredCurState, EWeaponHandlingState NextState, UAnimMontage* TargetAM, ESeekerMontageSlot TargetMontageSlot);

public:
	UFUNCTION(Server, Reliable)
	void Server_RestKey();

	// State
	UPROPERTY(Replicated)
	bool bIsAiming = false;

	// ===============
	// 시커 타입 체크 함수들 (GS_Character의 ECharacterType 사용)
	// ===============
	UFUNCTION(BlueprintPure, Category = "Seeker Type")
	bool IsChan() const { return GetCharacterType() == ECharacterType::Chan; }

	UFUNCTION(BlueprintPure, Category = "Seeker Type")
	bool IsAres() const { return GetCharacterType() == ECharacterType::Ares; }

	UFUNCTION(BlueprintPure, Category = "Seeker Type")
	bool IsMerci() const { return GetCharacterType() == ECharacterType::Merci; }

	// 근접/원거리 체크 (하위 호환성)
	UFUNCTION(BlueprintPure, Category = "Seeker Type")
	bool IsMeleeSeeker() const { return IsChan() || IsAres(); }

	UFUNCTION(BlueprintPure, Category = "Seeker Type")
	bool IsRangedSeeker() const { return IsMerci(); }

	// ==========================================
	// 가디언 감지 HUD 시스템
	// ==========================================

	/** 가디언이 시커를 감지했을 때 호출 (public 인터페이스) */
	UFUNCTION(BlueprintCallable, Category = "Detection")
	void OnDetectedByGuardian(bool bIsDetected);

	/** 현재 가디언에게 감지되었는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Detection")
	bool IsDetectedByGuardian() const { return bIsDetectedByGuardian; }

	/** 화면 중앙 근접도 설정 (서버에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Detection")
	void SetDetectionIntensity(float Intensity);

	/** 현재 감지 강도 확인 */
	UFUNCTION(BlueprintPure, Category = "Detection")
	float GetDetectionIntensity() const { return DetectionIntensity; }

	/** 감지 HUD 위젯 인스턴스 (블루프린트 접근용) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI|Detection")
	class UUserWidget* DetectionHUDWidget;

	/** 감지 상태 변경 시 HUD 업데이트 */
	void UpdateDetectionHUD();

private:
	/** 감지 상태 변경 시 시각적/청각적 효과 업데이트 */
	void UpdateDetectionEffects();

	/** 화면 중앙 근접도 기반 포스트 프로세스 효과 업데이트 */
	void UpdateDetectionPostProcessEffect(float Intensity);

	// ==========================================
	// 빈사 (Dying) 상태 시스템
	// ==========================================
public:
	/** 빈사 상태 진입 (서버에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Dying")
	void EnterDyingState();

	/** 빈사 상태 해제 - 구조 완료 또는 사망 시 */
	UFUNCTION(BlueprintCallable, Category = "Dying")
	void ExitDyingState(bool bWasRevived);

	/** 구조 완료 처리 - 25% HP 회복 */
	UFUNCTION(BlueprintCallable, Category = "Dying")
	void OnRevived();

	/** 빈사 상태 시간 만료 - 실제 사망 처리 */
	UFUNCTION()
	void OnDyingTimeExpired();

	/** 현재 빈사 상태인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Dying")
	bool IsInDyingState() const { return bIsInDyingState; }

	/** 남은 빈사 시간 확인 */
	UFUNCTION(BlueprintPure, Category = "Dying")
	float GetDyingTimeRemaining() const { return DyingTimeRemaining; }

	/** 최대 빈사 시간 (90초) */
	UFUNCTION(BlueprintPure, Category = "Dying")
	float GetMaxDyingTime() const { return MaxDyingTime; }

	/** 구조 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Dying")
	bool IsBeingRevived() const { return bIsBeingRevived; }

	/** 현재 구조 진행도 (0~1) */
	UFUNCTION(BlueprintPure, Category = "Dying")
	float GetReviveProgress() const { return ReviveProgress; }

	/** 현재 빈사 횟수 확인 */
	UFUNCTION(BlueprintPure, Category = "Dying")
	int32 GetCurrentDyingCount() const { return CurrentDyingCount; }

	/** 최대 빈사 허용 횟수 확인 */
	UFUNCTION(BlueprintPure, Category = "Dying")
	int32 GetMaxDyingCount() const { return MaxDyingCount; }

	/** 구조 시작 (서버 RPC) */
	UFUNCTION(Server, Reliable, Category = "Dying")
	void Server_StartRevive(AGS_Seeker* Reviver);

	/** 구조 취소 (서버 RPC) */
	UFUNCTION(Server, Reliable, Category = "Dying")
	void Server_CancelRevive();

	/** 구조 진행도 업데이트 (서버에서 호출) */
	void UpdateReviveProgress(float DeltaTime);

	/** 구조 완료 처리 (서버에서 호출) */
	void CompleteRevive();

	/** 빈사 상태 변화 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Dying")
	FOnDyingStateChanged OnDyingStateChanged;

	/** 구조 진행도 변화 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Dying")
	FOnReviveProgressChanged OnReviveProgressChanged;

	/** 구조하는 플레이어 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "Dying")
	TWeakObjectPtr<AGS_Seeker> CurrentReviver;

protected:
	/** 빈사 상태 업데이트 (Tick에서 호출) */
	void UpdateDyingState(float DeltaTime);

	/** 빈사 상태 화면 효과 업데이트 */
	void UpdateDyingPostProcessEffect();

	/** 진행도 감소 시작 */
	UFUNCTION()
	void StartReviveDecay();

	/** 진행도 감소 중지 */
	UFUNCTION()
	void StopReviveDecay();

	/** 진행도 감소 Tick */
	UFUNCTION()
	void OnReviveDecayTick();

private:
	// ========================================
	// 빈사 상태 변수들
	// ========================================

	/** 빈사 상태 여부 */
	UPROPERTY(ReplicatedUsing = OnRep_IsInDyingState)
	bool bIsInDyingState = false;

	/** 남은 빈사 시간 (초) */
	UPROPERTY(Replicated)
	float DyingTimeRemaining = 0.0f;

	/** 최대 빈사 시간 (90초) */
	UPROPERTY(EditDefaultsOnly, Category = "Dying", meta = (ClampMin = "10.0", ClampMax = "300.0"))
	float MaxDyingTime = 90.0f;

	/** 최대 빈사 허용 횟수 (기본 3번, 3번째에 사망) */
	UPROPERTY(EditDefaultsOnly, Category = "Dying", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxDyingCount = 3;

	/** 현재 빈사 횟수 (누적) */
	UPROPERTY(Replicated)
	int32 CurrentDyingCount = 0;

	/** 구조 중인지 여부 */
	UPROPERTY(ReplicatedUsing = OnRep_IsBeingRevived)
	bool bIsBeingRevived = false;

	/** 구조 진행도 (0~1) */
	UPROPERTY(ReplicatedUsing = OnRep_ReviveProgress)
	float ReviveProgress = 0.0f;

	/** 구조에 필요한 시간 (8초) */
	UPROPERTY(EditDefaultsOnly, Category = "Dying", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float ReviveTime = 8.0f;

	/** 구조 후 회복되는 HP 비율 (25%) */
	UPROPERTY(EditDefaultsOnly, Category = "Dying", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ReviveHealthPercent = 0.25f;

	/** 빈사 상태 PostProcess 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Dying|Effects")
	UPostProcessComponent* DyingPostProcessComp;

	/** 빈사 상태 PostProcess 머티리얼 */
	UPROPERTY(EditDefaultsOnly, Category = "Dying|Effects")
	UMaterialInterface* DyingEffectMaterial;

	/** 빈사 상태 동적 머티리얼 */
	UPROPERTY()
	UMaterialInstanceDynamic* DyingDynamicMaterial;

	/** 빈사 진입 전 Gait 저장 */
	EGait GaitBeforeDying;

	// ========================================
	// 구조 진행도 감소 시스템
	// ========================================

	/** 진행도 감소 타이머 핸들 */
	FTimerHandle ReviveDecayTimerHandle;

	/** 진행도 감소 속도 (초당 감소량, 0.25 = 4초에 100% -> 0%) */
	UPROPERTY(EditDefaultsOnly, Category = "Dying", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ReviveDecayRate = 0.25f;

	/** 진행도가 감소 중인지 여부 (서버 전용) */
	bool bIsReviveDecaying = false;

	// OnRep 함수들
	UFUNCTION()
	void OnRep_IsInDyingState();

	UFUNCTION()
	void OnRep_IsBeingRevived();

	UFUNCTION()
	void OnRep_ReviveProgress();
};