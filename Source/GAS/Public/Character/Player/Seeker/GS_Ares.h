// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "GS_Ares.generated.h"

class AGS_SwordAuraProjectile;
class UAkAudioEvent;

UCLASS()
class GAS_API AGS_Ares : public AGS_Seeker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Ares();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Projectile
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Projectile")
	TSubclassOf<AGS_SwordAuraProjectile> AresProjectileClass;

	/*virtual void OnComboAttack() override;*/

	virtual void ServerAttackMontage() override;

	virtual void MulticastPlayComboSection() override;

	// ===============
	// 전용 공격 사운드
	// ===============
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* SwordSwingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* SwordSwingStopEvent;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Attack")
	UAkAudioEvent* FinalAttackExtraSound;  // 4번째 공격 추가 사운드

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Attack")
	class UNiagaraSystem* FinalAttackHitVFX; // 4번째 공격 추가 VFX

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Voice")
	UAkAudioEvent* AttackVoiceSound;
	
	// 사운드 중첩 방지를 위한 현재 재생 중인 사운드 ID
	UPROPERTY()
	int32 CurrentSoundPlayingID = -1;

	// ===============
	// 타격 처리 관련
	// ===============
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAttackHit(int32 ComboIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
