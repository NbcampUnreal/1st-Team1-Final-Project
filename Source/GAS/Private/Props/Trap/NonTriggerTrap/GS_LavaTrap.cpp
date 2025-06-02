#include "Props/Trap/NonTriggerTrap/GS_LavaTrap.h"
#include "Engine/DamageEvents.h"

AGS_LavaTrap::AGS_LavaTrap()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f;
}

void AGS_LavaTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority())
	{
		return;
	}

	CheckAndActivateFireEffects();
}

void AGS_LavaTrap::CheckAndActivateFireEffects_Implementation()
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
