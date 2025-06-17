#include "Props/Trap/GS_TrapManager.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Props/Trap/TrapMotion/GS_TrapMotionCompBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"


AGS_TrapManager::AGS_TrapManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetLoadOnClient = true;
	
}


void AGS_TrapManager::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] TrapManager BeginPlay"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CLIENT] TrapManager BeginPlay"));
	}

	//switch (GetNetMode())
	//{
	//case NM_Standalone:
	//	UE_LOG(LogTemp, Warning, TEXT("[TrapManager] NetMode: Standalone"));
	//	break;
	//case NM_Client:
	//	UE_LOG(LogTemp, Warning, TEXT("[TrapManager] NetMode: Client"));
	//	break;
	//case NM_ListenServer:
	//	UE_LOG(LogTemp, Warning, TEXT("[TrapManager] NetMode: ListenServer"));
	//	break;
	//case NM_DedicatedServer:
	//	UE_LOG(LogTemp, Warning, TEXT("[TrapManager] NetMode: DedicatedServer"));
	//	break;
	//}
	GetWorld()->GetTimerManager().SetTimer(
		MotionLoopTimerHandle,
		this,
		&AGS_TrapManager::TimerTrapManager,
		MotionInterval,
		true
	);
}

void AGS_TrapManager::TimerTrapManager()
{
	for (auto& WeakTrap : RegisteredTraps)
	{
		if (AGS_TrapBase* Trap = WeakTrap.Get())
		{
			if (!Trap->CanStartMotion())
			{
				continue;
			}
			if (UGS_TrapMotionCompBase* MotionComp = Trap->GetValidMotionComponent())
			{
				if (HasAuthority())
				{
					//UE_LOG(LogTemp, Warning, TEXT("[TrapManager] Do All Trap Motion succeeded"));
					MotionComp->StartMotion();
				}
				//else
				//{
				//	//UE_LOG(LogTemp, Warning, TEXT("[client]ClientLerpUpdate called"));
				//	MotionComp->ClientLerpUpdate();
				//}
			}

		}
	}
}


void AGS_TrapManager::RegisterTrap(AGS_TrapBase* Trap)
{
	if (!RegisteredTraps.Contains(Trap))
	{
		RegisteredTraps.Add(Trap);
		//UE_LOG(LogTemp, Warning, TEXT("[TrapManager] Registered Trap: %s"), *Trap->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TrapManager] Tried to register null Trap!"));
	}
}

void AGS_TrapManager::UnregisterTrap(AGS_TrapBase* Trap)
{
	RegisteredTraps.Remove(Trap);
}

//void AGS_TrapManager::DoAllTrapMotion()
//{
//	for (auto& WeakTrap : RegisteredTraps)
//	{
//		if (AGS_TrapBase* Trap = WeakTrap.Get())
//		{
//			if (!Trap->CanStartMotion())
//			{
//				continue;
//			}
//			if (UGS_TrapMotionCompBase* MotionComp = Trap->GetValidMotionComponent())
//			{
//				UE_LOG(LogTemp, Warning, TEXT("[TrapManager] Do All Trap Motion succeeded"));
//				MotionComp->StartMotion();
//			}
//				
//		}
//	}
//}
//
//void AGS_TrapManager::DoClientLerpUpdate()
//{
//	for (auto& WeakTrap : RegisteredTraps)
//	{
//		if (AGS_TrapBase* Trap = WeakTrap.Get())
//		{
//			if (!Trap->CanStartMotion())
//			{
//				continue;
//			}
//			if (UGS_TrapMotionCompBase* MotionComp = Trap->GetValidMotionComponent())
//			{
//				UE_LOG(LogTemp, Warning, TEXT("[TrapManager] Do All Trap Motion succeeded"));
//				MotionComp->ClientLerpUpdate();
//			}
//
//		}
//	}
//}


void AGS_TrapManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_TrapManager, RegisteredTraps);
}