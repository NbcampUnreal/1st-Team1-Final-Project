// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_Timer.h"
#include "Components/TextBlock.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GameState/GS_BossLevelGS.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/CanvasPanelSlot.h"

void UGS_Timer::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TimerText)
	{
		return;
	}

	TimerText->SetText(FText::FromString(TEXT("--:--")));
	TimerText->SetColorAndOpacity(FSlateColor(NormalColor));

	GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &UGS_Timer::TryInitialize, 0.2f, true);
	TryInitialize();
}

void UGS_Timer::TryInitialize()
{
	AGS_InGameGS* InGameGS = GetWorld()->GetGameState<AGS_InGameGS>();
	AGS_BossLevelGS* BossGS = GetWorld()->GetGameState<AGS_BossLevelGS>();

	if (InGameGS || BossGS)
	{
		CachedInGameGS = InGameGS;
		CachedBossLevelGS = BossGS;

		GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("UGS_Timer: GameState found and initialized successfully."));
	}
}

void UGS_Timer::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!TimerText)
	{
		return;
	}

	float RemainingTime = -1.0f;
	FText FormattedTime = FText::FromString(TEXT("--:--"));

	if (CachedInGameGS.IsValid())
	{
		const float TimeSinceUpdate = GetWorld()->GetTimeSeconds() - CachedInGameGS->LastServerTimeUpdate;
		// 서버의 현재 시간을 로컬에서 추정
		const float EstimatedServerCurrentTime = CachedInGameGS->CurrentTime + TimeSinceUpdate;
		// 추정된 남은 시간
		RemainingTime = FMath::Max(0.0f, CachedInGameGS->TotalGameTime - EstimatedServerCurrentTime);
	}
	else if (CachedBossLevelGS.IsValid())
	{
		const float TimeSinceUpdate = GetWorld()->GetTimeSeconds() - CachedBossLevelGS->LastServerTimeUpdate;
		const float EstimatedServerCurrentTime = CachedBossLevelGS->BossCurrentTime + TimeSinceUpdate;
		RemainingTime = FMath::Max(0.0f, CachedBossLevelGS->BossTotalTime - EstimatedServerCurrentTime);
	}

	if (RemainingTime >= 0.0f)
    {
        const int32 TotalSeconds = FMath::FloorToInt(RemainingTime);

        // 마지막으로 표시한 시간과 다를 때만 텍스트 업데이트
        if (LastDisplayedSeconds != TotalSeconds)
        {
            LastDisplayedSeconds = TotalSeconds;

            const int32 Min = TotalSeconds / 60;
            const int32 Sec = TotalSeconds % 60;
            FormattedTime = FText::FromString(FString::Printf(TEXT("%02d:%02d"), Min, Sec));

            TimerText->SetText(FormattedTime);
			UpdateTextColor(RemainingTime);
        }
    }
    else 
    {
        // 타이머가 끝났을 때 기본 텍스트로 한 번만 설정
        if (LastDisplayedSeconds != -1)
        {
            LastDisplayedSeconds = -1;
            TimerText->SetText(FText::FromString(TEXT("--:--")));
        }
    }
}

void UGS_Timer::UpdateTextColor(float RemainingTimeInSeconds)
{
	if (!TimerText) return;

	FLinearColor TargetColor = NormalColor;
	if (RemainingTimeInSeconds <= UrgentTimeThreshold)
	{
		TargetColor = UrgentColor;
		StartShakeEffect();
	}
	else if (RemainingTimeInSeconds <= CriticalTimeThreshold)
	{
		TargetColor = CriticalColor;
		StopShakeEffect();
	}
	else if (RemainingTimeInSeconds <= WarningTimeThreshold)
	{
		TargetColor = WarningColor;
		StopShakeEffect();
	}
	else
	{
		TargetColor = NormalColor;
		StopShakeEffect();
	}

	TimerText->SetColorAndOpacity(FSlateColor(TargetColor));
}

void UGS_Timer::StartShakeEffect()
{
	if (bIsShaking) return;

	bIsShaking = true;
	ShakeTimer = 0.0f;

	if (!bOriginalPositionSaved)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerText->Slot))
		{
			OriginalPosition = CanvasSlot->GetPosition();
			bOriginalPositionSaved = true;
		}
	}

	GetWorld()->GetTimerManager().SetTimer(ShakeTimerHandle, this, &UGS_Timer::UpdateShakeEffect, 1.0f / 60.0f, true);
}

void UGS_Timer::StopShakeEffect()
{
	if (!bIsShaking) return;

	bIsShaking = false;
	GetWorld()->GetTimerManager().ClearTimer(ShakeTimerHandle);

	if (bOriginalPositionSaved)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerText->Slot))
		{
			CanvasSlot->SetPosition(OriginalPosition);
		}
	}
}

void UGS_Timer::UpdateShakeEffect()
{
	if (!bIsShaking || !TimerText) return;

	ShakeTimer += GetWorld()->GetDeltaSeconds();

	const float ShakeOffsetX = FMath::Sin(ShakeTimer * ShakeSpeed * 10.0f) * ShakeIntensity; // 헤더의 속도 변수 사용
	const float ShakeOffsetY = FMath::Cos(ShakeTimer * ShakeSpeed * 10.0f) * ShakeIntensity;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerText->Slot))
	{
		const FVector2D NewPosition = OriginalPosition + FVector2D(ShakeOffsetX, ShakeOffsetY);
		CanvasSlot->SetPosition(NewPosition);
	}
}