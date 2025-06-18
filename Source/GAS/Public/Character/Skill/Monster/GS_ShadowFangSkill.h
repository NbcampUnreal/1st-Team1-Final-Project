// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/Monster/GS_MonsterSkillBase.h"
#include "GS_ShadowFangSkill.generated.h"


class AGS_BuffZone;

UCLASS()
class GAS_API UGS_ShadowFangSkill : public UGS_MonsterSkillBase
{
	GENERATED_BODY()

public:
	UGS_ShadowFangSkill();

	virtual void ActiveSkill() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	TSubclassOf<AGS_BuffZone> BuffZoneClass;

private:
	void SpawnBuffZone();
};
