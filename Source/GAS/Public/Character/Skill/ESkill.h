#pragma once

#include "CoreMinimal.h"
#include "ESkill.generated.h"

UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Ready,
	Moving,
	Aiming,
	Ultimate,
	Rolling
};