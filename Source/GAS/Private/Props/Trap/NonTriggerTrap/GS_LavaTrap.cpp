#include "Props/Trap/NonTriggerTrap/GS_LavaTrap.h"
#include "Engine/DamageEvents.h"

AGS_LavaTrap::AGS_LavaTrap()
{
}


void AGS_LavaTrap::StartLavaLoop(AGS_Seeker* Seeker)
{
	if (!Seeker || !HasAuthority())
	{
		return;
	}

	if (ActiveLavaTimers.Contains(Seeker))
	{
		return;
	}


	HandleTrapDamage(Seeker);

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;

	TimerDel.BindLambda([this, Seeker]()
		{
			CheckLavaLoop(Seeker);
		});

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 1.0f, false);
	ActiveLavaTimers.Add(Seeker, TimerHandle);
}

void AGS_LavaTrap::CheckLavaLoop(AGS_Seeker* Seeker)
{
	if (!Seeker || !HasAuthority())
	{
		ActiveLavaTimers.Remove(Seeker);
		return;
	}

	ActiveLavaTimers.Remove(Seeker);
	
	if (DamageBoxComp->IsOverlappingActor(Seeker))
	{
		StartLavaLoop(Seeker);
	}

}
