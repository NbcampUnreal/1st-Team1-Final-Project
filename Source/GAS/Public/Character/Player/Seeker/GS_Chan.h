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
	// 스킬 사운드
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Skill")
	UAkAudioEvent* AimingSkillSlamSound;

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
	// 사운드 멀티캐스트 관련
	// ===============
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopAttackSound();

	// ===============
	// 스킬 사운드
	// ===============

	// [Widget]
	void SetChanAimingSkillBarWidget(UGS_ChanAimingSkillBar* Widget) { ChanAimingSkillBarWidget = Widget; }

	UFUNCTION(Client, Reliable)
	void Client_UpdateChanAimingSkillBar(float Stamina);

	UFUNCTION(Client, Reliable)
	void Client_ChanAimingSkillBar(bool bShow);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	// Knockback Collision (KCY)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UltimateSkill")
	UCapsuleComponent* UltimateCollision;

	UFUNCTION()
	void OnUltimateOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

private:
	UGS_ChanAimingSkillBar* ChanAimingSkillBarWidget;
};