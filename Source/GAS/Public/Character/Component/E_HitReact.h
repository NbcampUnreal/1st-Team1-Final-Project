#pragma once

#include "CoreMinimal.h"
#include "E_HitReact.generated.h"

UENUM(BlueprintType)
enum class EHitReactType : uint8
{
	Interrupt,
	Additive,
	DamageOnly,
	TypeNum,
};