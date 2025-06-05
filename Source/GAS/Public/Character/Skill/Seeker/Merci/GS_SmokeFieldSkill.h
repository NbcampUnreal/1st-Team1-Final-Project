// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "GS_SmokeFieldSkill.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_SmokeFieldSkill : public AGS_FieldSkillActor
{
	GENERATED_BODY()

public:
	AGS_SmokeFieldSkill();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void ApplyFieldEffectToMonster(AGS_Monster* Target);
	virtual void RemoveFieldEffectFromMonster(AGS_Monster* Target);
	virtual void ApplyFieldEffectToGuardian(AGS_Guardian* Target);
	virtual void RemoveFieldEffectFromGuardian(AGS_Guardian* Target);

private:
	bool bShouldDescendToGround = false;
	FVector TargetGroundLocation;
	float DescendSpeed = 0.1f; // 초당 200cm 정도로 내려오기
	float GroundZ = -1.0f; // 기본 높이
};
