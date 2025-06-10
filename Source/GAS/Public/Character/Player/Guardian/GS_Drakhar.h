#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GS_Drakhar.generated.h"

class AGS_DrakharProjectile;
class UAkAudioEvent;
class UAkComponent;

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

	// === Wwise 사운드 재생 함수들 ===
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "콤보 공격 사운드 재생"))
	void PlayComboAttackSound();

	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "대시 스킬 사운드 재생"))
	void PlayDashSkillSound();

	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "지진 스킬 사운드 재생"))
	void PlayEarthquakeSkillSound();

	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "드래곤 분노 스킬 사운드 재생"))
	void PlayDraconicFurySkillSound();

	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (DisplayName = "드래곤 분노 투사체 사운드 재생"))
	void PlayDraconicProjectileSound(const FVector& Location);

	// === Multicast 사운드 RPC 함수들 ===
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

	// === Wwise 관련 헬퍼 함수들 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
	UAkComponent* GetOrCreateAkComponent();
};
