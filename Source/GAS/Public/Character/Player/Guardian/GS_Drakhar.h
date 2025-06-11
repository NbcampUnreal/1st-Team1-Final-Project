#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GS_Drakhar.generated.h"

class AGS_DrakharProjectile;
class UAkAudioEvent;
class UAkComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UArrowComponent;

UCLASS()
class GAS_API AGS_Drakhar : public AGS_Guardian
{
	GENERATED_BODY()
	
public:
	AGS_Drakhar();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//[combo attack variables]
	UPROPERTY(ReplicatedUsing=OnRep_CanCombo)
	bool bCanCombo;
	
	bool bClientCanCombo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> Projectile;

	//[Draconic Fury Variables]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> DraconicProjectile;

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

	// === 날기 관련 사운드 이벤트들 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "FlyStart"))
	UAkAudioEvent* FlyStartSoundEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Drakhar", meta = (DisplayName = "FlyEnd"))
	UAkAudioEvent* FlyEndSoundEvent;

	// === 나이아가라 VFX 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush Ribbon VFX"))
	UNiagaraSystem* WingRushRibbonVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveWingRushVFXComponent;

	// === VFX 위치 제어용 화살표 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush VFX Spawn Point"))
	UArrowComponent* WingRushVFXSpawnPoint;

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

	//[Fly Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCStartCtrl();

	UFUNCTION(Server, Reliable)
	void ServerRPCStopCtrl();

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

	// === 날기 사운드 재생 함수 ===
	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Flying Start"))
	void PlayFlyStartSound();

	UFUNCTION(BlueprintCallable, Category = "Sound", meta = (DisplayName = "Flying End"))
	void PlayFlyEndSound();

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

	// === 날기 사운드 Multicast RPC 함수 ===
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFlyStartSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFlyEndSound();

	// === 나이아가라 VFX 관련 ===
	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "WingRush VFX 시작"))
	void StartWingRushVFX();

	UFUNCTION(BlueprintCallable, Category = "VFX", meta = (DisplayName = "WingRush VFX 종료"))
	void StopWingRushVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartWingRushVFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopWingRushVFX();
	
private:
	//[NEW COMBO ATTACK]
	FName ComboAttackSectionName;
	FName DefaultComboAttackSectionName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	TObjectPtr<UAnimMontage> ComboAttackMontage;
	
	//[dash skill]
	UPROPERTY()
	TSet<AGS_Character*> DamagedCharacters;
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

	//[Draconic Fury]
	TArray<FTransform> DraconicFuryTargetArray;
	void GetRandomDraconicFuryTarget();

	// === Wwise 관련 헬퍼 함수 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
	UAkComponent* GetOrCreateAkComponent();
};
