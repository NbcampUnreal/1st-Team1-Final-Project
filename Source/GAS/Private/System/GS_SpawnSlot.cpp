#include "System/GS_SpawnSlot.h"

AGS_SpawnSlot::AGS_SpawnSlot()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	ForRole = EPlayerRole::PR_None;
	SlotIndex = 0;
}

