// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/Player/GS_Player.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "GS_Seeker.generated.h"

class UGS_SkillInputHandlerComp;
class UPostProcessComponent;
class UMaterialInterface;
class UGS_StatComp;

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
	bool IsDraw;
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

	// Montage Set
	UPROPERTY()
	float NewPlayRate = 0.5f;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetNewPlayRate(float PlayRate);
	
	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects")
	UPostProcessComponent* LowHealthPostProcessComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects")
	UMaterialInterface* LowHealthEffectMaterial;
	
	UFUNCTION()
	void HandleLowHealthEffect(UGS_StatComp* InStatComp);

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
private:
	UPROPERTY()
	TArray<class AGS_Monster*> NearbyMonsters;

public:
	// 몬스터가 전투 음악 시작/중지를 요청할 때 호출
	UFUNCTION(BlueprintCallable)
	void AddCombatMonster(class AGS_Monster* Monster);
	
	UFUNCTION(BlueprintCallable)
	void RemoveCombatMonster(class AGS_Monster* Monster);

private:
	void StartCombatMusic();
	
	UFUNCTION(Client, Unreliable)
	void ClientRPCStopCombatMusic();
	void UpdateCombatMusicState();

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

private :
	UPROPERTY(VisibleAnywhere, Category="State")
	FSeekerState SeekerState;

	// ================
	// LowHP 스크린 효과
	// ================
	UPROPERTY(EditDefaultsOnly, Category="Effects", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LowHealthThresholdRatio = 0.3f;
};
