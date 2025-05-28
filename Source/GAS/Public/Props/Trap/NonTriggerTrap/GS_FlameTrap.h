#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "GS_FlameTrap.generated.h"


UCLASS()
class GAS_API AGS_FlameTrap : public AGS_NonTrigTrapBase
{
	GENERATED_BODY()
	
public:
	AGS_FlameTrap();


	virtual void HandleTrapDamage(AActor* OtherActor) override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Trap|Damage")
	float DamageInterval = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Trap|Damage")
	int32 DamageCount = 3;

	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveDamageTimers;
	TMap<TWeakObjectPtr<AActor>, int32> DamageCounter;

};
