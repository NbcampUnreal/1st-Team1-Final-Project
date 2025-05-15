#include "Character/Player/Monster/GS_SmallClaw.h"

AGS_SmallClaw::AGS_SmallClaw()
{
}

void AGS_SmallClaw::BeginPlay()
{
	Super::BeginPlay();
	
	if (SmallClawClickSound)
	{
		ClickSoundEvent = SmallClawClickSound;
	}

	if (SmallClawMoveSound)
	{
		MoveSoundEvent = SmallClawMoveSound;
	}
} 