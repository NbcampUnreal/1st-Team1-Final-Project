#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_Drakhar.generated.h"

class UGS_DrakharFeverGauge;
class AGS_DrakharProjectile;
class UAkAudioEvent;
class UAkComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UArrowComponent;
class UGS_CameraShakeComponent;
class UGS_FootManagerComponent;
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
	
	// === 어스퀘이크 카메라 쉐이크 정보 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Earthquake", meta = (DisplayName = "Earthquake Camera Shake Info"))
	FGS_CameraShakeInfo EarthquakeShakeInfo;

	//[combo attack variables]
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_CanCombo)
	bool bCanCombo;
	bool bClientCanCombo;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsAttacking;
	

	//[Draconic Fury Variables]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> DraconicProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> FeverDraconicProjectile;

	
	// === Wwise 사운드 이벤트들 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "NormalAttack"))
	UAkAudioEvent* ComboAttackSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "WingRush"))
	UAkAudioEvent* DashSkillSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "Earthquake"))
	UAkAudioEvent* EarthquakeSkillSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "DraconicFury"))
	UAkAudioEvent* DraconicFurySkillSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "DraconicFury Projectile"))
	UAkAudioEvent* DraconicProjectileSoundEvent;

	// === 히트 사운드 이벤트 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "AttackHit"))
	UAkAudioEvent* AttackHitSoundEvent;

	// === 피버모드 사운드 이벤트 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "FeverModeStart"))
	UAkAudioEvent* FeverModeStartSoundEvent;

	// === 나이아가라 VFX 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush Ribbon VFX"))
	UNiagaraSystem* WingRushRibbonVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever WingRush Ribbon VFX"))
	UNiagaraSystem* FeverWingRushRibbonVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveWingRushVFXComponent;

	// === DustVFX 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Dust VFX"))
	UNiagaraSystem* DustVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever Dust VFX"))
	UNiagaraSystem* FeverDustVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveDustVFXComponent;

	// === VFX 위치 제어용 화살표 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush VFX Spawn Point"))
	UArrowComponent* WingRushVFXSpawnPoint;

	FOnCurrentFeverGaugeChangedDelegate OnCurrentFeverGaugeChanged;
	
	// === 어스퀘이크 VFX 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Ground Crack VFX"))
	UNiagaraSystem* GroundCrackVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveGroundCrackVFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Dust Cloud VFX"))
	UNiagaraSystem* DustCloudVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveDustCloudVFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Earthquake Impact VFX"))
	UNiagaraSystem* EarthquakeImpactVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Fever Earthquake Impact VFX"))
	UNiagaraSystem* FeverEarthquakeImpactVFX;

	// === 어스퀘이크 VFX 위치 제어용 화살표 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Earthquake", meta = (DisplayName = "Earthquake VFX Spawn Point"))
	UArrowComponent* EarthquakeVFXSpawnPoint;

	// === DraconicFury 충돌 이펙트 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Projectile Impact VFX"))
	UNiagaraSystem* DraconicProjectileImpactVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|DraconicFury", meta = (DisplayName = "Projectile Explosion VFX"))
	UNiagaraSystem* DraconicProjectileExplosionVFX;

	// === DraconicFury 충돌 사운드 이벤트 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|DraconicFury", meta = (DisplayName = "Projectile Impact Sound"))
	UAkAudioEvent* DraconicProjectileImpactSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|DraconicFury", meta = (DisplayName = "Projectile Explosion Sound"))
	UAkAudioEvent* DraconicProjectileExplosionSoundEvent;

	// === 피버모드 발자국 VFX ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever Footstep VFX"))
	UNiagaraSystem* FeverFootstepVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Flying Dust VFX"))
	UNiagaraSystem* FlyingDustVFX;
	
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveFlyingDustVFXComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Flying Dust VFX Trace Distance"))
	float FlyingDustTraceDistance;

	// === 공격 히트 VFX ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Normal Attack Hit VFX"))
	UNiagaraSystem* NormalAttackHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Fever Attack Hit VFX"))
	UNiagaraSystem* FeverAttackHitVFX;

	//[Input Binding Function]
	virtual void Ctrl() override;

	virtual void CtrlStop() override;

	virtual void LeftMouse() override;
	
	virtual void RightMouse() override;
	
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
	
	UFUNCTION()
	void OnRep_CanCombo();

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
	
	//[Fever Mode]
	FORCEINLINE float GetCurrentFeverGauge() const { return CurrentFeverGauge; }
	FORCEINLINE float GetMaxFeverGauge() const { return MaxFeverGauge; }
	FORCEINLINE bool GetIsFeverMode() const {return IsFeverMode; }
	
	void SetFeverGaugeWidget(UGS_DrakharFeverGauge* InDrakharFeverGaugeWidget);

	//UFUNCTION(NetMulticast, Reliable)
	void SetFeverGauge(float InValue);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCFeverMontagePlay();
	
	//new skill
	void FeverComoLastAttack();
	
	//max fever gauge
	void StartFeverMode();
	//when fever gauge > 0
	void DecreaseFeverGauge();
	//minus 1 values per one seconds
	void MinusFeverGaugeValue();

	// === Wwise 사운드 재생 함수 ===
	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Normal Attack"))
	void PlayComboAttackSound();

	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Wing Rush"))
	void PlayDashSkillSound();

	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Earthquake"))
	void PlayEarthquakeSkillSound();

	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Draconic Fury"))
	void PlayDraconicFurySkillSound();

	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Draconic Fury Projectile"))
	void PlayDraconicProjectileSound(const FVector& Location);

	// === 히트 사운드 재생 함수 ===
	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Attack Hit"))
	void PlayAttackHitSound();

	// === 피버모드 사운드 재생 함수 ===
	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Fever Mode Start"))
	void PlayFeverModeStartSound();

	// === Multicast 사운드 RPC 함수 ===
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayComboAttackSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDashSkillSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayEarthquakeSkillSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDraconicFurySkillSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDraconicProjectileSound(const FVector& Location);

	// === 히트 사운드 Multicast RPC 함수 ===
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackHitSound();

	// === 피버모드 사운드 Multicast RPC 함수 ===
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFeverModeStartSound();

	// === 나이아가라 VFX 관련 ===
	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "WingRush VFX Start"))
	void StartWingRushVFX();

	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "WingRush VFX End"))
	void StopWingRushVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartWingRushVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopWingRushVFX();

	// === DustVFX 관련 ===
	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "Dust VFX Start"))
	void StartDustVFX();

	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "Dust VFX End"))
	void StopDustVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartDustVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopDustVFX();

	// === 어스퀘이크 VFX 관련 함수 ===
	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "Earthquake Ground Crack VFX Start"))
	void StartGroundCrackVFX();

	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "Earthquake Ground Crack VFX Stop"))
	void StopGroundCrackVFX();

	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "Earthquake Dust Cloud VFX Start"))
	void StartDustCloudVFX();

	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "Earthquake Dust Cloud VFX Stop"))
	void StopDustCloudVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartGroundCrackVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopGroundCrackVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartDustCloudVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopDustCloudVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayEarthquakeImpactVFX(const FVector& ImpactLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFeverEarthquakeImpactVFX(const FVector& ImpactLocation);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_PlayAttackHitVFX(FVector ImpactPoint);

	// === 날기 이벤트 ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Fly", meta = (DisplayName = "On Fly Start"))
	void BP_OnFlyStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Fly", meta = (DisplayName = "On Fly End"))
	void BP_OnFlyEnd();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_OnFlyStart();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_OnFlyEnd();

	// === 궁극기 이벤트 ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Ultimate", meta = (DisplayName = "On Ultimate Start"))
	void BP_OnUltimateStart();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_OnUltimateStart();

	// === 어스퀘이크 이벤트 ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Earthquake", meta = (DisplayName = "On Earthquake Start"))
	void BP_OnEarthquakeStart();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_OnEarthquakeStart();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UGS_FootManagerComponent* FootManagerComponent;

	// === 피버모드 오버레이 머티리얼 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode")
	UMaterialInterface* FeverModeOverlayMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* FeverModeOverlayMID;

	// === 피버모드 오버레이 파라미터 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode")
	float FeverOverlayIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|FeverMode")
	FLinearColor FeverOverlayColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

private:
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
	FVector DashStartLocation;
	FVector DashEndLocation;
	FVector DashDirection;
	
	float DashPower;
	float DashInterpAlpha;
	float DashDuration;

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

	float PillarForwardOffset = 300.f;
	float PillarSideSpacing = 400.f;
	float PillarRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	TObjectPtr<UAnimMontage> FeverOnMontage;

	//[draconic fury]
	void GetRandomDraconicFuryTarget();
	void EndDraconicFury();
	
	// 사운드 중복 재생 방지
	bool bDraconicFurySoundPlayed;
	
	// === Wwise 관련 헬퍼 함수 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
	UAkComponent* GetOrCreateAkComponent();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ApplyFeverModeOverlay();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_RemoveFeverModeOverlay();
	
	UFUNCTION()
	void OnRep_FeverGauge();

	void ApplyFeverModeOverlay();
	void RemoveFeverModeOverlay();

	UFUNCTION()
	void OnRep_IsFeverMode();
};
