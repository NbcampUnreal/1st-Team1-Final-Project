// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Character/GS_DyingStateWidget.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UGS_DyingStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기에는 숨김
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGS_DyingStateWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 빈사 상태 시커가 유효한지 확인
	if (!DyingSeeker.IsValid())
	{
		return;
	}

	// 빈사 상태가 아니면 숨김
	if (!DyingSeeker->IsInDyingState())
	{
		HideWidget();
		return;
	}

	// UI 업데이트
	UpdateTimerUI(DyingSeeker->GetDyingTimeRemaining(), DyingSeeker->GetMaxDyingTime());
	UpdateReviveUI(DyingSeeker->GetReviveProgress(), DyingSeeker->IsBeingRevived());
}

void UGS_DyingStateWidget::NativeDestruct()
{
	UnbindDelegates();
	Super::NativeDestruct();
}

void UGS_DyingStateWidget::SetDyingSeeker(AGS_Seeker* InSeeker)
{
	// 기존 델리게이트 해제
	UnbindDelegates();

	DyingSeeker = InSeeker;

	// 새 델리게이트 바인딩
	if (IsValid(InSeeker))
	{
		BindDelegates();

		// 현재 상태에 따라 표시/숨김
		if (InSeeker->IsInDyingState())
		{
			ShowWidget();
		}
		else
		{
			HideWidget();
		}
	}
}

void UGS_DyingStateWidget::ShowWidget()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UGS_DyingStateWidget::HideWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGS_DyingStateWidget::OnDyingStateChanged(bool bIsDying, float TimeRemaining)
{
	if (bIsDying)
	{
		ShowWidget();
	}
	else
	{
		HideWidget();
	}
}

void UGS_DyingStateWidget::OnReviveProgressChanged(float Progress)
{
	if (DyingSeeker.IsValid())
	{
		UpdateReviveUI(Progress, DyingSeeker->IsBeingRevived());
	}
}

void UGS_DyingStateWidget::UpdateTimerUI(float TimeRemaining, float MaxTime)
{
	// 타이머 바 업데이트
	if (DyingTimerBar)
	{
		float TimerPercent = FMath::Clamp(TimeRemaining / MaxTime, 0.0f, 1.0f);
		DyingTimerBar->SetPercent(TimerPercent);

		// 시간에 따른 색상 변경
		if (TimeRemaining <= CriticalTimeThreshold)
		{
			DyingTimerBar->SetFillColorAndOpacity(CriticalColor);
		}
		else
		{
			DyingTimerBar->SetFillColorAndOpacity(NormalColor);
		}
	}

	// 타이머 텍스트 업데이트
	if (DyingTimerText)
	{
		int32 RemainingSeconds = FMath::CeilToInt(TimeRemaining);
		int32 Minutes = RemainingSeconds / 60;
		int32 Seconds = RemainingSeconds % 60;

		FText TimerTextValue;
		if (Minutes > 0)
		{
			TimerTextValue = FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds));
		}
		else
		{
			TimerTextValue = FText::FromString(FString::Printf(TEXT("%d"), Seconds));
		}
		DyingTimerText->SetText(TimerTextValue);

		// 시간에 따른 색상 변경
		if (TimeRemaining <= CriticalTimeThreshold)
		{
			DyingTimerText->SetColorAndOpacity(CriticalColor);
		}
		else
		{
			DyingTimerText->SetColorAndOpacity(NormalColor);
		}
	}
}

void UGS_DyingStateWidget::UpdateReviveUI(float Progress, bool bIsBeingRevived)
{
	// 구조 진행도 바 업데이트
	if (ReviveProgressBar)
	{
		if (bIsBeingRevived)
		{
			ReviveProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
			ReviveProgressBar->SetPercent(Progress);
			ReviveProgressBar->SetFillColorAndOpacity(ReviveColor);
		}
		else
		{
			ReviveProgressBar->SetVisibility(ESlateVisibility::Collapsed);
			ReviveProgressBar->SetPercent(0.0f);
		}
	}

	// 구조 상태 텍스트 업데이트
	if (ReviveStatusText)
	{
		if (bIsBeingRevived)
		{
			ReviveStatusText->SetVisibility(ESlateVisibility::HitTestInvisible);
			int32 ProgressPercent = FMath::RoundToInt(Progress * 100.0f);
			ReviveStatusText->SetText(FText::FromString(FString::Printf(TEXT("구조 중... %d%%"), ProgressPercent)));
			ReviveStatusText->SetColorAndOpacity(ReviveColor);
		}
		else
		{
			ReviveStatusText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGS_DyingStateWidget::BindDelegates()
{
	if (DyingSeeker.IsValid())
	{
		DyingSeeker->OnDyingStateChanged.AddDynamic(this, &UGS_DyingStateWidget::OnDyingStateChanged);
		DyingSeeker->OnReviveProgressChanged.AddDynamic(this, &UGS_DyingStateWidget::OnReviveProgressChanged);
	}
}

void UGS_DyingStateWidget::UnbindDelegates()
{
	if (DyingSeeker.IsValid())
	{
		DyingSeeker->OnDyingStateChanged.RemoveDynamic(this, &UGS_DyingStateWidget::OnDyingStateChanged);
		DyingSeeker->OnReviveProgressChanged.RemoveDynamic(this, &UGS_DyingStateWidget::OnReviveProgressChanged);
	}
}

