// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_Timer.h"
#include "Components/TextBlock.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GameState/GS_BossLevelGS.h"

void UGS_Timer::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TimerText) return;
	
	if (AGS_InGameGS* GS = GetWorld()->GetGameState<AGS_InGameGS>())
	{
		// 델리게이트에 바인딩
		GS->OnTimerUpdated.AddDynamic(this, &UGS_Timer::HandleTimeUpdated);
		TimerText->SetText(GS->GetFormattedTime());
	}

	if (AGS_BossLevelGS* GS = GetWorld()->GetGameState<AGS_BossLevelGS>())
	{
		GS->OnBossTimerUpdatedDelegate.AddDynamic(this, &UGS_Timer::HandleTimeUpdated);
		TimerText->SetText(GS->GetFormattedBossTime());
	}
}

void UGS_Timer::HandleTimeUpdated(const FText& NewTime)
{
	if (TimerText)
	{
		TimerText->SetText(NewTime);
		double startSeconds = FPlatformTime::Seconds();
	}
}
