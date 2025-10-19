// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "NiagaraComponent.h"
#include "Animation/Character/E_SeekerAnim.h"
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

// 충돌 사운드 타입 열거형
UENUM(BlueprintType)
enum class ECollisionSoundType : uint8
{
	Wall,
	Monster, 
	Guardian
};

UCLASS()
class GAS_API AGS_Seeker : public AGS_Player
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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 상수들
	static const FName HPRatioParamName;
	static const FName EffectIntensityParamName;

	// Post Process 설정
	void InitializeCameraManager();
	void UpdatePostProcessEffect(float EffectStrength);

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
};