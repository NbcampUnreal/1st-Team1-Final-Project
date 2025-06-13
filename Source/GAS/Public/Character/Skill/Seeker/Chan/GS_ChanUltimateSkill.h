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
	virtual void ExecuteSkillEffect() override;

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
	float KnockbackRadius = 300.0f; // 넉백 반경

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float KnockbackForce = 800.0f; // 넉백 힘

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float Damage = 100.0f; // 스킬 데미지

	// 돌진 상태 관리
	bool bIsCharging = false;
	FVector ChargeStartLocation;
	FVector ChargeDirection;
	FTimerHandle ChargeTimerHandle;

	// 이미 맞은 적들 추적 (중복 피해 방지)
	TSet<AActor*> HitActors;

	void StartCharge();
	void UpdateCharge();
	void EndCharge();
	void CheckAndApplyDamage();

	// 가디언용 넉백 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Settings", meta = (AllowPrivateAccess = "true"))
	float GuardianKnockbackForce = 300.0f; // 가디언 넉백 힘

	bool IsEnemyTeam(class AGS_Player* Player1, class AGS_Player* Player2);
	void ApplyDamageToTarget(class AGS_Player* Target, float DamageAmount);
};
