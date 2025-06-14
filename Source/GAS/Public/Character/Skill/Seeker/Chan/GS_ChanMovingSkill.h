// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_SeekerSkillBase.h"
#include "GS_ChanMovingSkill.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanMovingSkill : public UGS_SeekerSkillBase
{
	GENERATED_BODY()

public:
	UGS_ChanMovingSkill();

	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
	virtual void ExecuteSkillEffect() override;

protected:
	// 공격
	virtual void ApplyEffectToDungeonMonster(AGS_Monster* Target) override;
	virtual void ApplyEffectToGuardian(AGS_Guardian* Target) override;

private:
	// ===============================
	// VFX 재생 함수들 (멀티캐스트) - 데이터 테이블 기반
	// ===============================
	
	// 스킬 시전 VFX 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkillVFX(FVector SpawnLocation, FRotator SpawnRotation);

	// 스킬 범위 VFX 재생
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySkillRangeVFX(FVector SpawnLocation, float Radius);
};
