// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_MonsterSkillBase.generated.h"


class AGS_Monster;

UCLASS()
class GAS_API UGS_MonsterSkillBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Cooltime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Damage;

	UPROPERTY()
	TObjectPtr<AGS_Monster> OwnerCharacter;
	
	void InitSkill(AGS_Monster* InOwner);
	virtual void ActiveSkill();
	
};
