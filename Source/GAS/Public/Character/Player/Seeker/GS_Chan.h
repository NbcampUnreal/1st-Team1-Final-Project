// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Iris/ReplicationSystem/ReplicationSystemTypes.h"
#include "GS_Chan.generated.h"

class AGS_WeaponShield;
class AGS_WeaponAxe;
class UGS_ChanAimingSkillBar;
class UAkAudioEvent;

UCLASS()
class GAS_API AGS_Chan : public AGS_Seeker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Chan();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void OnComboAttack() override;

	virtual void MulticastPlayComboSection() override;

	// Aim Skill
	/*void OnReadyAimSkill();*/
	void OnJumpAttackSkill();
	void OffJumpAttackSkill();
	void ToIdle();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DrawSkillRange(FVector InLocation, float InRadius, FColor InColor, float InLifetime);

	/*// AnimInstnace Slot State Value 
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsUpperBodySlot(bool bUpperBodySlot);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetIsFullBodySlot(bool bFullBodySlot);*/

	// Control State Value
	UFUNCTION()
	void SetMoveControlValue(bool bMoveForward, bool bMoveRight);
	UFUNCTION()
	void SetLookControlValue(bool bLookUp, bool bLookRight);
	/*UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetMustTurnInPlace(bool MustTurn);*/

	// ===============
	// 전용 공격 사운드
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* AxeSwingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* AxeSwingStopEvent;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* FinalAttackExtraSound;  // 4번째 공격 추가 사운드

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Attack")
	class UNiagaraSystem* FinalAttackHitVFX; // 4번째 공격 추가 VFX

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Voice")
	UAkAudioEvent* AttackVoiceSound;

	// ===============
	// 공격 사운드 리셋 관련
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float AttackSoundResetTime = 1.0f;

	FTimerHandle AttackSoundResetTimerHandle;

	UFUNCTION()
	void ResetAttackSoundSequence();

	// ===============
	// 타격 처리 관련
	// ===============
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAttackHit(int32 ComboIndex);

	// ===============
	// 스킬 사운드
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Skill")
	UAkAudioEvent* AimingSkillStartSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Skill")
	UAkAudioEvent* AimingSkillSlamSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Skill")
	UAkAudioEvent* MovingSkillSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Skill")
	UAkAudioEvent* UltimateSkillSound;


	// 스킬 사운드 재생 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkillSound(UAkAudioEvent* SoundToPlay);

	// [Widget]
	void SetChanAimingSkillBarWidget(UGS_ChanAimingSkillBar* Widget) { ChanAimingSkillBarWidget = Widget; }

	UFUNCTION(Client, Reliable)
	void Client_UpdateChanAimingSkillBar(float Stamina);

	UFUNCTION(Client, Reliable)
	void Client_ChanAimingSkillBar(bool bShow);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UGS_ChanAimingSkillBar* ChanAimingSkillBarWidget;

	// === Wwise 헬퍼 함수 ===
	UAkComponent* GetOrCreateAkComponent();
};