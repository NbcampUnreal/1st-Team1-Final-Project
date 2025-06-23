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

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Voice")
	UAkAudioEvent* AttackVoiceSound;

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
};