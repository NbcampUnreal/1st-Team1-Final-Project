#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "GS_BearTrap.generated.h"


UCLASS()
class GAS_API AGS_BearTrap : public AGS_NonTrigTrapBase
{
	GENERATED_BODY()
	

	virtual void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;


	virtual EHitReactType GetHitReactType() const override;
};
