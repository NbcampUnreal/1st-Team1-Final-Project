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

	/*virtual void OnComboAttack() override;*/

	virtual void MulticastPlayComboSection() override;

	// Aim Skill
	/*void OnReadyAimSkill();*/
	void OnJumpAttackSkill();
	void OffJumpAttackSkill();
	void ToIdle();

	// ===============
	// 찬 전용 공격 시스템
	// ===============
	
	// 찬 전용 공격 VFX
	UPROPERTY(EditDefaultsOnly, Category = "Chan|VFX|Attack", meta = (DisplayName = "4번째 공격 타격 VFX"))
	class UNiagaraSystem* FinalAttackHitVFX;

	// ===============
	// 찬 전용 스킬 시스템
	// ===============
	
	// 찬 전용 방패 슬램 스킬 범위 표시
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DrawSkillRange(FVector InLocation, float InRadius, FColor InColor, float InLifetime);

	// 찬 전용 콤보 공격 타격 처리
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAttackHit(int32 ComboIndex);

	// 찬 전용 궁극기 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chan|UltimateSkill", meta = (DisplayName = "궁극기 충돌 컴포넌트"))
	UCapsuleComponent* UltimateCollision;

	// 찬 전용 궁극기 오버랩 처리 Knockback Collision (KCY)
	UFUNCTION()
	void OnUltimateOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// ===============
	// 찬 전용 UI 시스템
	// ===============
	
	// 찬 전용 방패 들기 스킬 UI 위젯
	void SetChanAimingSkillBarWidget(UGS_ChanAimingSkillBar* Widget) { ChanAimingSkillBarWidget = Widget; }

	UFUNCTION(Client, Reliable)
	void Client_UpdateChanAimingSkillBar(float Stamina);

	UFUNCTION(Client, Reliable)
	void Client_ChanAimingSkillBar(bool bShow);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	// Damage handling with audio feedback
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

private:
	UGS_ChanAimingSkillBar* ChanAimingSkillBarWidget;
};