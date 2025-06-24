#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_Drakhar.generated.h"

class UGS_DrakharFeverGauge;
class AGS_DrakharProjectile;
class UGS_DrakharVFXComponent;
class UGS_DrakharSFXComponent;
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
	void SetFeverGauge(float InValue);
	void ResetIsAttackingDuringFeverMode();
	void StartIsAttackingTimer();
	
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

	//[Healing System]
	FTimerHandle HealthRegenTimer;
	FTimerHandle HealthDelayTimer;
	void BeginHealRegeneration();
	void HealRegeneration();
	void StopHealRegeneration();

	// === Multicast RPCs delegated to components ===
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayComboAttackSound();
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayDashSkillSound();
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayEarthquakeSkillSound();
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayDraconicFurySkillSound();
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayDraconicProjectileSound(const FVector& Location);
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayAttackHitSound();
	UFUNCTION(NetMulticast, Unreliable) void MulticastPlayFeverModeStartSound();
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
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnFlyStart();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnFlyEnd();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnUltimateStart();
	UFUNCTION(NetMulticast, Unreliable) void MulticastRPC_OnEarthquakeStart();
	
	// === Blueprint Events ===
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Fly", meta = (DisplayName = "On Fly Start"))
	void BP_OnFlyStart();
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Fly", meta = (DisplayName = "On Fly End"))
	void BP_OnFlyEnd();
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Ultimate", meta = (DisplayName = "On Ultimate Start"))
	void BP_OnUltimateStart();
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill|Earthquake", meta = (DisplayName = "On Earthquake Start"))
	void BP_OnEarthquakeStart();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UGS_DrakharVFXComponent* VFXComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UGS_DrakharSFXComponent* SFXComponent;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Fever")
	UAkAudioEvent* FeverModeStartSoundEvent;
	
	FORCEINLINE UGS_DrakharVFXComponent* GetVFXComponent() const { return VFXComponent; }
	FORCEINLINE UGS_DrakharSFXComponent* GetSFXComponent() const { return SFXComponent; }

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
};
