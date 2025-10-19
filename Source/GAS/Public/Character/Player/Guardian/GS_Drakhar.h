#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_Drakhar.generated.h"

class UGS_DrakharFeverGauge;
class AGS_DrakharProjectile;
class UGS_DrakharVFXComponent;
class UGS_DrakharAudioComponent;
class UGS_FootManagerComponent;
class UArrowComponent;
class UNiagaraSystem;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UAkAudioEvent;
class AGS_EarthquakeEffect;
struct FGS_CameraShakeInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentFeverGaugeChangedDelegate, float);

UCLASS()
class GAS_API AGS_Drakhar : public AGS_Guardian
{
	GENERATED_BODY()
	
public:
	AGS_Drakhar();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void OnDamageStart() override;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGS_EarthquakeEffect> GC_EarthquakeEffect;
	
	//[combo attack variables]
	UPROPERTY(Replicated)
	bool bCanCombo;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsAttacking;
	
	//[Draconic Fury Variables]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> DraconicProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> FeverDraconicProjectile;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* BloodEffectSystem;

	//[fever mode]
	FOnCurrentFeverGaugeChangedDelegate OnCurrentFeverGaugeChanged;
	UPROPERTY()
	bool bIsAttckingDuringFever;
	FTimerHandle ResetAttackTimer;

	//[health regeneration]
	bool bIsDamaged = false;
	
	//[Input Binding Function]
	virtual void Ctrl() override;
	virtual void CtrlStop() override;
	virtual void LeftMouse() override;
	virtual void RightMouse() override;
	
	virtual void OnAttackHit(AGS_Character* HitCharacter) override;
	virtual void OnFeverGaugeUpdate(float DeltaGauge) override;
	virtual void OnQuitSkill() override;
	
	//[Attack Functions]
	virtual void MeleeAttackCheck() override;
	
	//[COMBO ATTACK]
	void SetNextComboAttackSection(FName InSectionName);
	void ResetComboAttackSection();
	void PlayComboAttackMontage();
	
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& PayLoad);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCNewComboAttack();

	UFUNCTION(Server,Reliable)
	void ServerRPCShootEnergy();
	
	UFUNCTION(Server, Reliable)
	void ServerRPCResetValue();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCComboAttack();

	void ComboLastAttack();
	
	//[Dash Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCDoDash(float DeltaTime);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCEndDash();

	UFUNCTION(Server, Reliable)
	void ServerRPCCalculateDashLocation();
	
	UFUNCTION()
	void DashAttackCheck();

	//[Earthquake Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCEarthquakeAttackCheck();
	
	//[DraconicFury Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCSpawnDraconicFury();

	UFUNCTION(Server, Reliable)
	void ServerRPC_BeginDraconicFury();

	// === DraconicFury 투사체 충돌 처리 ===
	UFUNCTION(BlueprintCallable, Category = "DraconicFury")
	void HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDraconicProjectileImpactEffects(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter);

	//[Fly Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCStartCtrl();
	UFUNCTION(Server, Reliable)
	void ServerRPCStopCtrl();

	void StartCtrl() override;
	void StopCtrl() override;
	
	
	//[Fever Mode]
	FORCEINLINE float GetCurrentFeverGauge() const { return CurrentFeverGauge; }
	FORCEINLINE float GetMaxFeverGauge() const { return MaxFeverGauge; }
	FORCEINLINE bool GetIsFeverMode() const {return IsFeverMode; }
	
	void SetFeverGaugeWidget(UGS_DrakharFeverGauge* InDrakharFeverGaugeWidget);
	void SetFeverGauge(float InValue);
	void ResetIsAttackingDuringFeverMode();
	void StartIsAttackingTimer();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCFeverMontagePlay();
	
	//new skill
	void FeverComoLastAttack();
	void PlayDelayedComboFinisherSounds();

	//max fever gauge
	void StartFeverMode();
	//when fever gauge > 0
	void DecreaseFeverGauge();
	//minus 1 values per one seconds
	void MinusFeverGaugeValue();

	//[Healing System]
	FTimerHandle HealthRegenTimer;
	FTimerHandle HealthDelayTimer;
	void BeginHealRegeneration();
	void HealRegeneration();
	void StopHealRegeneration();

	// === Multicast RPCs for VFX only ===
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayFeverModeEndEffects();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStartWingRushVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStopWingRushVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStartDustVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStopDustVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStartGroundCrackVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStopGroundCrackVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStartDustCloudVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastStopDustCloudVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayEarthquakeImpactVFX(const FVector& ImpactLocation);
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayFeverEarthquakeImpactVFX(const FVector& ImpactLocation);
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_PlayAttackHitVFX(FVector ImpactPoint);
	//UFUNCTION(NetMulticast, Unreliable) void MulticastPlayFeverModeEndVFX();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnFlyStart();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnFlyEnd();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnUltimateStart();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnEarthquakeStart();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnFeverModeStart();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnFeverModeEnd();
	
	UFUNCTION(NetMulticast, Reliable) void Multicast_PlayBloodEffect(FVector HitLocation, FVector HitNormal, float Scale);
	
	// === Blueprint Events ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Fly", meta = (DisplayName = "On Fly Start"))
	void BP_OnFlyStart();
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Fly", meta = (DisplayName = "On Fly End"))
	void BP_OnFlyEnd();
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Ultimate", meta = (DisplayName = "On Ultimate Start"))
	void BP_OnUltimateStart();
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Earthquake", meta = (DisplayName = "On Earthquake Start"))
	void BP_OnEarthquakeStart();
	UFUNCTION(BlueprintImplementableEvent, Category = "FeverMode", meta = (DisplayName = "On Fever Mode Start"))
	void BP_OnFeverModeStart();
	UFUNCTION(BlueprintImplementableEvent, Category = "FeverMode", meta = (DisplayName = "On Fever Mode End"))
	void BP_OnFeverModeEnd();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UGS_DrakharVFXComponent* DrakharVFXComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UGS_DrakharAudioComponent* AudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UGS_FootManagerComponent* FootManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush VFX Spawn Point"))
	UArrowComponent* WingRushVFXSpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Earthquake", meta = (DisplayName = "Earthquake VFX Spawn Point"))
	UArrowComponent* EarthquakeVFXSpawnPoint;
	
public:
	//dash skill public variables for vfx component
	FVector DashStartLocation;
	FVector DashEndLocation;
	float DashPower;
	float DashDuration;

	// For component access
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Earthquake", meta = (DisplayName = "Earthquake Camera Shake Info"))
	FGS_CameraShakeInfo EarthquakeShakeInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush Ribbon VFX"))
	UNiagaraSystem* WingRushRibbonVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever WingRush Ribbon VFX"))
	UNiagaraSystem* FeverWingRushRibbonVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Dust VFX"))
	UNiagaraSystem* DustVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever Dust VFX"))
	UNiagaraSystem* FeverDustVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Ground Crack VFX"))
	UNiagaraSystem* GroundCrackVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Dust Cloud VFX"))
	UNiagaraSystem* DustCloudVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Earthquake Impact VFX"))
	UNiagaraSystem* EarthquakeImpactVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Fever Earthquake Impact VFX"))
	UNiagaraSystem* FeverEarthquakeImpactVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Projectile Impact VFX"))
	UNiagaraSystem* DraconicProjectileImpactVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Projectile Explosion VFX"))
	UNiagaraSystem* DraconicProjectileExplosionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Fever Projectile Impact VFX"))
	UNiagaraSystem* FeverDraconicProjectileImpactVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Fever Projectile Explosion VFX"))
	UNiagaraSystem* FeverDraconicProjectileExplosionVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Normal Mode Indicator VFX"))
	UNiagaraSystem* DraconicFuryIndicatorVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Fever Mode Indicator VFX"))
	UNiagaraSystem* FeverDraconicFuryIndicatorVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever Footstep VFX"))
	UNiagaraSystem* FeverFootstepVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Flying Dust VFX"))
	UNiagaraSystem* FlyingDustVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Flying Dust VFX Trace Distance"))
	float FlyingDustTraceDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Normal Attack Hit VFX"))
	UNiagaraSystem* NormalAttackHitVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever Attack Hit VFX"))
	UNiagaraSystem* FeverAttackHitVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode")
	UMaterialInterface* FeverModeOverlayMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode")
	float FeverOverlayIntensity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode")
	FLinearColor FeverOverlayColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode", meta = (DisplayName = "Fever Mode End VFX"))
	UNiagaraSystem* FeverModeEndVFX;

	// === Wwise Sound Events ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Combo")
	UAkAudioEvent* ComboAttackSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Skill")
	UAkAudioEvent* DashSkillSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Skill")
	UAkAudioEvent* EarthquakeSkillSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Skill")
	UAkAudioEvent* DraconicFurySkillSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Skill")
	UAkAudioEvent* DraconicProjectileSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Impact")
	UAkAudioEvent* DraconicProjectileImpactSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Impact")
	UAkAudioEvent* DraconicProjectileExplosionSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Impact")
	UAkAudioEvent* AttackHitSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Combo")
	UAkAudioEvent* ComboFinisherSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Fever")
	UAkAudioEvent* FeverModeStartSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Fever")
	UAkAudioEvent* FeverModeEndSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Fever")
	UAkAudioEvent* FeverModeStateSoundEvent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Impact")
	UAkAudioEvent* HurtSoundEvent;
	
	FORCEINLINE UGS_DrakharVFXComponent* GetDrakharVFXComponent() const { return DrakharVFXComponent; }
	FORCEINLINE UGS_DrakharAudioComponent* GetAudioComponent() const { return AudioComponent; }

	// 궁극기 타겟 배열 접근자 (인디케이터용)
	FORCEINLINE const TArray<FTransform>& GetDraconicFuryTargetArray() const { return DraconicFuryTargetArray; }
	FORCEINLINE const FVector& GetFeverModeDraconicFurySpawnLocation() const { return FeverModeDraconicFurySpawnLocation; }

	// 궁극기 타겟 생성 함수
	void GenerateDraconicFuryTargets();

private:
	// === 카메라 효과 상태 관리 ===

	// 카메라 효과 단계
	enum class ECameraEffectPhase : uint8
	{
		None,      // 효과 없음
		ZoomIn,    // 줌인 단계
		ZoomOut,   // 줌아웃 단계 ("쾅" 효과)
		Restore    // 원래 상태로 복귀
	};

	ECameraEffectPhase CurrentCameraEffectPhase = ECameraEffectPhase::None;

	// 카메라 효과 상수
	static constexpr float CAMERA_UPDATE_INTERVAL = 0.01f;      // 타이머 간격 (10ms)
	static constexpr float FOV_TOLERANCE = 0.5f;                 // FOV 도달 판정 허용 오차
	static constexpr float ARM_LENGTH_TOLERANCE = 5.0f;          // Arm Length 도달 판정 허용 오차
	static constexpr float FINAL_FOV_TOLERANCE = 0.1f;           // 최종 FOV 복귀 판정 허용 오차
	static constexpr float FINAL_ARM_LENGTH_TOLERANCE = 1.0f;    // 최종 Arm Length 복귀 판정 허용 오차

	//move spring arm for flying
	float DefaultSpringArmLength;
	float TargetSpringArmLength;
	bool bIsFlying;
	
	//[NEW COMBO ATTACK]
	FName ComboAttackSectionName;
	FName DefaultComboAttackSectionName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	TObjectPtr<UAnimMontage> ComboAttackMontage;
	
	//[dash skill]
	UPROPERTY()
	TSet<AGS_Character*> DamagedCharactersFromDash;
	
	FVector DashDirection;
	float DashInterpAlpha;

	//[earthquake]
	float EarthquakePower;
	float EarthquakeRadius;

	//[DraconicFury]
	FTimerHandle FlyingTimerHandle;
	FTimerHandle DraconicAttackTimer;
	float FlyingPersistenceTime;
	float DraconicAttackPersistenceTime;

	TArray<FTransform> DraconicFuryTargetArray;
	FVector FeverModeDraconicFurySpawnLocation;

	//[Fever Mode]
	float MaxFeverGauge;

	UPROPERTY(ReplicatedUsing = OnRep_FeverGauge)
	float CurrentFeverGauge;

	UPROPERTY(ReplicatedUsing = OnRep_IsFeverMode)
	bool IsFeverMode;

	FTimerHandle FeverTimer;
	FTimerHandle FeverStateSoundDelayTimer;

	// === 카메라 줌 효과 설정 ===

	// 카메라 줌인 효과 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float FeverEndZoomInFOVMultiplier = 0.7f;  // FOV 줌인 비율 (기본 30% 줌인)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "0.1", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float FeverEndZoomInArmMultiplier = 0.6f;  // 카메라 암 줌인 비율 (기본 40% 가까이)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0"))
	float FeverEndZoomInSpeed = 30.0f;  // 줌인 속도

	// 카메라 줌아웃 효과 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", ClampMax = "1.5", UIMin = "1.0", UIMax = "1.5"))
	float FeverEndZoomOutFOVMultiplier = 1.05f;  // FOV 줌아웃 비율 (기본 5% 더 나감)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", ClampMax = "1.5", UIMin = "1.0", UIMax = "1.5"))
	float FeverEndZoomOutArmMultiplier = 1.1f;  // 카메라 암 줌아웃 비율 (기본 10% 더 멀리)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0"))
	float FeverEndZoomOutSpeed = 40.0f;  // 줌아웃 속도 (빠른 "쾅" 효과)

	// 카메라 복귀 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FeverModeEnd", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", ClampMax = "20.0", UIMin = "1.0", UIMax = "20.0"))
	float FeverEndCameraRestoreSpeed = 8.0f;  // 복귀 속도

	// 카메라 효과용 런타임 변수들
	float OriginalFOV = 90.0f;
	float TargetFOV = 90.0f;
	float OriginalArmLength = 500.0f;
	float TargetArmLength = 500.0f;
	FTimerHandle CameraZoomTimer;

	float PillarForwardOffset = 300.f;
	float PillarSideSpacing = 400.f;
	float PillarRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	TObjectPtr<UAnimMontage> FeverOnMontage;

	UPROPERTY()
	UMaterialInstanceDynamic* FeverModeOverlayMID;

	//[draconic fury]
	void GetRandomDraconicFuryTarget();
	void EndDraconicFury();
	
	UFUNCTION()
	void OnRep_FeverGauge();
	
	UFUNCTION()
	void OnRep_IsFeverMode();

	// 월드 컨텍스트 검증 함수 (레벨 전환 시 크래시 방지)
	bool IsWorldContextValid() const;

	// 타이머 정리 함수 (레벨 전환 시 크래시 방지)
	void SafeClearTimer(FTimerHandle& TimerHandle);

	// FeverModeStateSound 딜레이 재생 콜백
	UFUNCTION()
	void PlayFeverModeStateSoundDelayed();

	// === 카메라 효과 함수 ===

	// 카메라 줌인아웃 효과 시작
	void ApplyFeverModeEndCameraEffect();

	// 카메라 검증 유틸리티
	bool ValidateCameraEffect(APlayerController*& OutPC) const;

	// 통합 카메라 업데이트 함수
	UFUNCTION()
	void UpdateCameraEffect();

	// 카메라 효과 단계별 업데이트 함수
	void UpdateCameraZoomIn();
	void UpdateCameraZoomOut();
	void UpdateCameraRestore();

	// 다음 카메라 효과 단계로 전환
	void TransitionToNextCameraPhase();
};
