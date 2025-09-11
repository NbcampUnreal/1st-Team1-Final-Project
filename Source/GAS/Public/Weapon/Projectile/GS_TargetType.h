// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_TargetType.generated.h"

UENUM(BlueprintType)
enum class ETargetType : uint8
{
	Guardian		UMETA(DisplayName = "Guardian"),
	DungeonMonster	UMETA(DisplayName = "DungeonMonster"),
	Seeker			UMETA(DisplayName = "Seeker"),
	AetherExtractor	UMETA(DisplayName = "AetherExtractor"),
	Structure		UMETA(DisplayName = "Structure"),
	Skill			UMETA(DisplayName = "Skill")
}; 