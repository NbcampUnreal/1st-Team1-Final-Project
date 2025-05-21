// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_SkillSet.generated.h"

USTRUCT(BlueprintType)
struct GAS_API FGS_SkillSet : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterType CharacterType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_SkillBase> ReadySkill;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_SkillBase> AimingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_SkillBase> MovingSkill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGS_SkillBase> UltimateSkill;

	FGS_SkillSet()
		: CharacterType(ECharacterType::Chan)
		, ReadySkill(nullptr)
		, AimingSkill(nullptr)
		, MovingSkill(nullptr)
		, UltimateSkill(nullptr)
	{
	}
};
