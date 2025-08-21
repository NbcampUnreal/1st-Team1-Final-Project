// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_ChanUltimateSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanUltimateSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()
	
public:
	UGS_ChanUltimateSkill();
	virtual void ActiveSkill() override;
	virtual void OnSkillCanceledByDebuff() override;
	virtual void OnSkillAnimationEnd() override;
	virtual void InterruptSkill() override;

	void HandleUltimateCollision(AActor* HitActor, UPrimitiveComponent* HitComp);

protected:
	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Settings", meta = (AllowPrivateAccess = "true"))
	float ChargeDistance = 1000.0f; // 돌진 거리

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float ChargeSpeed = 1500.0f; // 돌진 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float LateralControlStrength = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float KnockbackRadius = 300.0f; // 넉백 반경

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float KnockbackForce = 2000.0f; // 넉백 힘

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float MaxLateralSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float Damage = 100.0f; // 스킬 데미지

	virtual void DeactiveSkill() override;

	// 돌진 상태 관리
	FVector ChargeStartLocation;
	FVector ChargeDirection;
	FTimerHandle ChargeTimerHandle;
	FTimerHandle ChargeUpdateTimerHandle;

	// 캐릭터가 움직일 틱 간격 (예: 60FPS)
	static constexpr float ChargeTickInterval = 0.016f;

	// 좌우 방향 (시작 시 기준)
	FVector CurrentLateralDirection;

	// 현재 입력된 좌우 입력값 (-1.0 ~ 1.0)
	float CurrentLateralInput = 0.0f;

	// 이미 맞은 적들 추적 (중복 피해 방지)
	TSet<AActor*> HitActors;

	void StartCharge();
	void EndCharge();

	bool bInStructureCrash = false;

	// 가디언용 넉백 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float GuardianKnockbackForce = 1300.0f; // 가디언 넉백 힘
};
