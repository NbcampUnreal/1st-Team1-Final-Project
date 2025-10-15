#include "Props/Trap/NonTriggerTrap/GS_FlameTrap.h"



void AGS_FlameTrap::FlameOnSeeker(AGS_Seeker* TargetSeeker)
{
	if (!TargetSeeker || !HasAuthority())
	{
		return;
	}

	if (FTimerHandle* Handle = BurningPlayers.Find(TargetSeeker))
	{
		GetWorld()->GetTimerManager().ClearTimer(*Handle);
	}
	else
	{
		Multicast_SeekerFlameOn(TargetSeeker);
	}


	FTimerHandle FlameTimer;
	FTimerDelegate TimerDel;
	TWeakObjectPtr<AGS_Seeker> WeakSeeker = TargetSeeker;

	TimerDel.BindLambda([this, WeakSeeker]()
		{
			if (WeakSeeker.IsValid())
			{
				DeactivFlame(WeakSeeker.Get());
			}
		});
	GetWorld()->GetTimerManager().SetTimer(FlameTimer, TimerDel, 3.0f, false);
	BurningPlayers.Add(TargetSeeker, FlameTimer);
}

void AGS_FlameTrap::DeactivFlame(AGS_Seeker* TargetSeeker)
{
	Multicast_SeekerFlameOff(TargetSeeker);
	if (BurningPlayers.Contains(TargetSeeker))
	{
		BurningPlayers.Remove(TargetSeeker);
	}
}

void AGS_FlameTrap::Multicast_SeekerFlameOn_Implementation(AGS_Seeker* TargetSeeker)
{
	if (IsValid(TargetSeeker) && IsValid(TargetSeeker->BodyLavaVFX))
	{
		TargetSeeker->BodyLavaVFX->Activate();
	}
}

void AGS_FlameTrap::Multicast_SeekerFlameOff_Implementation(AGS_Seeker* TargetSeeker)
{
	if (IsValid(TargetSeeker) && IsValid(TargetSeeker->BodyLavaVFX))
	{
		TargetSeeker->BodyLavaVFX->Deactivate();
	}

}
