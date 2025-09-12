#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"
#include "GS_RisingSpikeTrap.generated.h"


UCLASS()
class GAS_API AGS_RisingSpikeTrap : public AGS_TrigTrapBase
{
	GENERATED_BODY()

	virtual EHitReactType GetHitReactType() const override;
};
