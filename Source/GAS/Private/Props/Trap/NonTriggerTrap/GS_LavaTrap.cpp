#include "Props/Trap/NonTriggerTrap/GS_LavaTrap.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"


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
	else
	{
		OnSeekerExitLava(Seeker);
	}
}

//디버프 적용 해제 체크
void AGS_LavaTrap::OnSeekerExitLava(AGS_Seeker* Seeker)
{
	if (!IsValid(Seeker))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[LavaTrap] Seeker %s exited lava."), *GetNameSafe(Seeker));
	FTimerHandle GraceTimer;
	FTimerDelegate GraceDel;

	GraceDel.BindLambda([this, Seeker]()
		{
			if (!IsValid(Seeker))
			{
				return;
			}
			bool bStillInAnyLava = false;

			for (TActorIterator<AGS_LavaTrap> It(GetWorld()); It; ++It)
			{
				AGS_LavaTrap* LavaTrap = *It;
				if (IsValid(LavaTrap) && LavaTrap->DamageBoxComp->IsOverlappingActor(Seeker))
				{
					bStillInAnyLava = true;
					break;
				}
			}


			if (!bStillInAnyLava)
			{
				
				if (UGS_DebuffComp* DebuffComp = Seeker->FindComponentByClass<UGS_DebuffComp>())
				{
					const FTrapEffect& Effect = TrapData.Effect;
					//Stun
					if (Effect.bSlow)
					{
						DebuffComp->RemoveDebuff(EDebuffType::Slow);
					}
				}
			}
		}
	);
	
	GetWorld()->GetTimerManager().SetTimer(GraceTimer, GraceDel, 0.3f, false);
	
	
}