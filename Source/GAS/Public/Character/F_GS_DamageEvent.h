#pragma once

#include "Engine/DamageEvents.h"
#include "Character/Component/E_HitReact.h"
#include "F_GS_DamageEvent.generated.h"

USTRUCT(BlueprintType)
struct FGS_DamageEvent : public FDamageEvent
{
	GENERATED_BODY()
public:
	FGS_DamageEvent()
		: HitReactType(EHitReactType::DamageOnly)
	{}
	
	EHitReactType HitReactType;
};