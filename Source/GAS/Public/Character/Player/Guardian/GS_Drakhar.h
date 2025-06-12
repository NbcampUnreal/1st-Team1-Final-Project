#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_Drakhar.generated.h"

class AGS_DrakharProjectile;
class UAkAudioEvent;
class UAkComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UArrowComponent;
class UGS_CameraShakeComponent;
struct FGS_CameraShakeInfo;

UCLASS()
class GAS_API AGS_Drakhar : public AGS_Guardian
{
	GENERATED_BODY()
	
public:
	AGS_Drakhar();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGS_CameraShakeComponent> CameraShakeComponent;
	
	// === 어스퀘이크 카메라 쉐이크 정보 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Earthquake", meta = (DisplayName = "Earthquake Camera Shake Info"))
	FGS_CameraShakeInfo EarthquakeShakeInfo;

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

	// === DustVFX 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Drakhar", meta = (DisplayName = "Dust VFX"))
	UNiagaraSystem* DustVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveDustVFXComponent;

	// === VFX 위치 제어용 화살표 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Drakhar", meta = (DisplayName = "WingRush VFX Spawn Point"))
	UArrowComponent* WingRushVFXSpawnPoint;

	// === 어스퀘이크 VFX 시스템 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Ground Crack VFX"))
	UNiagaraSystem* GroundCrackVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveGroundCrackVFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Earthquake", meta = (DisplayName = "Dust Cloud VFX"))
	UNiagaraSystem* DustCloudVFX;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveDustCloudVFXComponent;

	// === 어스퀘이크 VFX 위치 제어용 화살표 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Earthquake", meta = (DisplayName = "Earthquake VFX Spawn Point"))
	UArrowComponent* EarthquakeVFXSpawnPoint;

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
	void EndDraconicFury();
	
	// 사운드 중복 재생 방지
	bool bDraconicFurySoundPlayed;

	// === Wwise 관련 헬퍼 함수 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
	UAkComponent* GetOrCreateAkComponent();
};
