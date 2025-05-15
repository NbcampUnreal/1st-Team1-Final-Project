#include "Character/Player/Monster/GS_SmallClaw.h"

AGS_SmallClaw::AGS_SmallClaw()
{
}

void AGS_SmallClaw::BeginPlay()
{
	Super::BeginPlay();
	
	// SmallClaw 전용 클릭 사운드를 기본 클릭 사운드로 설정
	if (SmallClawClickSound)
	{
		ClickSoundEvent = SmallClawClickSound;
	}
} 