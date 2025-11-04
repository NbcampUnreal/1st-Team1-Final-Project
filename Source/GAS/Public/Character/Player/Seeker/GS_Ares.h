// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "Curves/CurveFloat.h"
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
	// 사운드 시스템 (GS_SeekerAudioComponent로 통합됨)
	// ===============
	// 모든 사운드는 GS_SeekerAudioComponent에서 처리됨
	// - 콤보 공격 사운드: PlayAresComboAttackSound() 함수 사용
	// - 콤보 공격 사운드 (추가 사운드 포함): PlayAresComboAttackSoundWithExtra() 함수 사용
	// - 사운드 배열들은 GS_SeekerAudioComponent의 AresComboXXX 프로퍼티들에서 설정

	UPROPERTY(EditDefaultsOnly, Category = "VFX|Attack")
	class UNiagaraSystem* FinalAttackHitVFX; // 4번째 공격 추가 VFX
	
	// 사운드 중첩 방지를 위한 현재 재생 중인 사운드 ID
	UPROPERTY()
	int32 CurrentSoundPlayingID = -1;

	// ===============
	// 타격 처리 관련
	// ===============
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAttackHit(int32 ComboIndex);

	// Damage handling with audio feedback
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 아레스 대시 스킬 카메라 복원
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RestoreDashCameraZoom();

public:
	// 아레스 대시 스킬 카메라 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Moving")
	float MovingSkill_ZoomOutDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Moving")
	UCurveFloat* MovingSkill_CameraZoomCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Moving|Camera")
	bool MovingSkill_EnableMotionBlur = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Moving|Camera", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MovingSkill_MotionBlurPeakAmount = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Moving|Camera")
	UCurveFloat* MovingSkill_MotionBlurCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Moving|Camera", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MovingSkill_MotionBlurExponent = 1.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
