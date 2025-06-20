// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/Player/GS_Player.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Animation/Character/E_SeekerAnim.h"
#include "GS_Seeker.generated.h"

class UGS_SkillInputHandlerComp;
class UPostProcessComponent;
class UMaterialInterface;
class UGS_StatComp;
class AGS_PlayerState;
class UGS_DebuffVFXComponent;

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

UCLASS()
class GAS_API AGS_Seeker : public AGS_Player
{
	GENERATED_BODY()

public:
	AGS_Seeker();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// OnDeath 오버라이드 추가
	virtual void OnDeath() override;

	UFUNCTION(BlueprintCallable)
	void SetAimState(bool IsAim);

	UFUNCTION(BlueprintPure, Category = "State")
	bool GetAimState();

	UFUNCTION(BlueprintCallable)
	void SetDrawState(bool IsDraw);

	UFUNCTION(BlueprintPure, Category = "State")
	bool GetDrawState();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	UChildActorComponent* Weapon;

	UFUNCTION(Server, Reliable, Category = "State")
	void Server_SetSeekerGait(EGait Gait);

	UFUNCTION()
	void SetSeekerGait(EGait Gait);

	UPROPERTY(Replicated)
	bool CanChangeSeekerGait;

	UFUNCTION(BlueprintCallable, Category = "State")
	EGait GetSeekerGait();

	UFUNCTION(BlueprintCallable, Category = "State")
	EGait GetLastSeekerGait();

	// Combo
	UPROPERTY(Replicated)
	bool bComboEnded = true;
	
	UFUNCTION(Server, Reliable)
	void Server_ComboEnd(bool bComboEnd);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ComboEnd();

	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* ComboAnimMontage;

	UPROPERTY(Replicated)
	int32 CurrentComboIndex;
	
	UPROPERTY(Replicated)
	bool CanAcceptComboInput = true;

	bool bNextCombo = false;

	UFUNCTION(Server, Reliable)
	virtual void ServerAttackMontage();

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastPlayComboSection();

	/*UFUNCTION()
	void ComboInputOpen();*/
	UFUNCTION()
	void ComboInputOpen();
	
	UFUNCTION()
	void ComboInputClose();

	UFUNCTION()
	virtual void OnComboAttack();
	
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
	void AddCombatMonster(class AGS_Monster* Monster);
	
	UFUNCTION(BlueprintCallable)
	void RemoveCombatMonster(class AGS_Monster* Monster);

	// 새로운 몬스터 감지 시스템 (시커의 CombatTrigger)
	UFUNCTION()
	void OnCombatTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCombatTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:
	UPROPERTY()
	TArray<class AGS_Monster*> NearbyMonsters;

	void StartCombatMusic();
	
	UFUNCTION(Client, Unreliable)
	void ClientRPCStopCombatMusic();
	void UpdateCombatMusicState();

	// PlayerState 생존 상태 변경 핸들러
	UFUNCTION()
	void HandleAliveStatusChanged(AGS_PlayerState* ChangedPlayerState, bool bIsNowAlive);

protected:
	virtual void BeginPlay() override;

	// EndPlay 함수 선언 추가
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	UGS_SkillInputHandlerComp* SkillInputHandlerComponent;

	// 동적 머티리얼 파라미터 사용
	UPROPERTY()
	UMaterialInstanceDynamic* LowHealthDynamicMaterial;

	// 카메라 매니저 참조 추가
	UPROPERTY()
	APlayerCameraManager* LocalCameraManager;

	// 카메라 매니저 관련 함수
	void InitializeCameraManager();
	void UpdatePostProcessEffect(float EffectStrength);

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

	// 리플리케이션 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 머티리얼 파라미터 이름 상수
	static const FName HPRatioParamName;
	static const FName EffectIntensityParamName;

	UPROPERTY(ReplicatedUsing = OnRep_SeekerGait)
	EGait SeekerGait;

	UPROPERTY(Replicated)
	EGait LastSeekerGait;

	UFUNCTION()
	void OnRep_SeekerGait();

private :
	UPROPERTY(VisibleAnywhere, Category="State")
	FSeekerState SeekerState;



	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(EditDefaultsOnly, Category="Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LowHealthThresholdRatio = 0.3f;
};