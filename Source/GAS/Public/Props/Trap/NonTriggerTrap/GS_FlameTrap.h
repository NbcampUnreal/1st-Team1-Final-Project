#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/NonTriggerTrap/GS_NonTrigTrapBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GS_FlameTrap.generated.h"



UCLASS()
class GAS_API AGS_FlameTrap : public AGS_NonTrigTrapBase
{
	GENERATED_BODY()
	

public:
	TMap<AGS_Seeker*, FTimerHandle> BurningPlayers;

	UFUNCTION(BlueprintCallable)
	void FlameOnSeeker(AGS_Seeker* TargetSeeker);

	UFUNCTION()
	void DeactivFlame(AGS_Seeker* TargetSeeker);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SeekerFlameOn(AGS_Seeker* TargetSeeker);
	void Multicast_SeekerFlameOn_Implementation(AGS_Seeker* TargetSeeker);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SeekerFlameOff(AGS_Seeker* TargetSeeker);
	void Multicast_SeekerFlameOff_Implementation(AGS_Seeker* TargetSeeker);
};
