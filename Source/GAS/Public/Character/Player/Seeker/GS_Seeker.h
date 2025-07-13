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
class UGS_DebuffVFXComponent;
class AGS_Monster;

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
	void OnRep_SeekerGait();

	// AnimInstnace Slot State Value 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsUpperBodySlot(bool bUpperBodySlot);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsFullBodySlot(bool bFullBodySlot);
	
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

	UFUNCTION()
	virtual void OnComboAttack();

	// Control
	UFUNCTION()
	void SetMoveControlValue(bool bMoveForward, bool bMoveRight);
	UFUNCTION()
	void SetLookControlValue(bool bLookUp, bool bLookRight);

	// Replication Set
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Notify
	void CallDeactiveSkill(ESkillSlot Slot);

	// 스킬 사운드 재생 (모든 시커 캐릭터에서 사용)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkillSound(class UAkAudioEvent* SoundToPlay);

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
	
	UFUNCTION()
	void HandleLowHealthEffect(UGS_StatComp* InStatComp);

	// =======================
	// 디버프 VFX 컴포넌트
	// =======================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UGS_DebuffVFXComponent* DebuffVFXComponent;

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

	// 사운드 관련 헬퍼 함수
	class UAkComponent* GetOrCreateAkComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	UGS_SkillInputHandlerComp* SkillInputHandlerComponent;

	// 동적 머티리얼 파라미터 사용
	UPROPERTY()
	UMaterialInstanceDynamic* LowHealthDynamicMaterial;

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
};