#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "GS_LavaTrap.generated.h"


UCLASS()
class GAS_API AGS_LavaTrap : public AGS_NonTrigTrapBase
{
	GENERATED_BODY()

	

public:
	AGS_LavaTrap();


	virtual void HandleTrapDamage(AActor* OtherActor) override;
};
