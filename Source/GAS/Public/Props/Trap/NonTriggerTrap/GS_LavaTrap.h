#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GS_LavaTrap.generated.h"


UCLASS()
class GAS_API AGS_LavaTrap : public AGS_NonTrigTrapBase
{
	GENERATED_BODY()

	

public:

	AGS_LavaTrap();

	
	FTimerHandle LavaTickTimer;
	TMap<AActor*, FTimerHandle> ActiveLavaTimers;
	
	
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void StartLavaLoop(AGS_Seeker* Seeker);

	void CheckLavaLoop(AGS_Seeker* Seeker);
};
