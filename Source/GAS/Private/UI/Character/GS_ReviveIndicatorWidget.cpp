// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Character/GS_ReviveIndicatorWidget.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/GS_PlayerState.h"

void UGS_ReviveIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기에는 숨김
	SetVisibility(ESlateVisibility::Collapsed);

	// 안내 텍스트 초기화
	if (InstructionText)
	{
		InstructionText->SetText(FText::FromString(TEXT("E키를 눌러 구조")));
	}
}

void UGS_ReviveIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 위젯이 보이는 상태이고 타겟이 유효할 때만 진행도 업데이트
	if (GetVisibility() != ESlateVisibility::Collapsed && CurrentTarget.IsValid())
	{
		float Progress = CurrentTarget->GetReviveProgress();
		if (Progress > 0.0f)
		{
			UpdateProgress(Progress);
		}
	}
}

void UGS_ReviveIndicatorWidget::StartRevive(AGS_Seeker* TargetSeeker)
{
	if (!IsValid(TargetSeeker))
	{
		return;
	}

	CurrentTarget = TargetSeeker;
	bIsReviving = true;

	// 위젯 표시
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 대상 이름 표시
	if (TargetNameText)
	{
		// PlayerState에서 이름 가져오기
		if (AGS_PlayerState* PS = TargetSeeker->GetPlayerState<AGS_PlayerState>())
		{
			TargetNameText->SetText(FText::FromString(PS->GetPlayerName()));
		}
		else
		{
			TargetNameText->SetText(FText::FromString(TargetSeeker->GetName()));
		}
	}

	// 안내 텍스트 숨김
	if (InstructionText)
	{
		InstructionText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 부상자의 현재 진행도를 가져와서 동기화
	float CurrentProgress = TargetSeeker->GetReviveProgress();

	// 프로그레스 바 표시 및 현재 진행도로 설정
	if (ReviveProgressBar)
	{
		ReviveProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		ReviveProgressBar->SetPercent(CurrentProgress);
		ReviveProgressBar->SetFillColorAndOpacity(ReviveColor);
	}

	if (ProgressText)
	{
		ProgressText->SetVisibility(ESlateVisibility::HitTestInvisible);
		int32 ProgressPercent = FMath::RoundToInt(CurrentProgress * 100.0f);
		ProgressText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), ProgressPercent)));
	}
}

void UGS_ReviveIndicatorWidget::StopRevive()
{
	bIsReviving = false;
	CurrentTarget = nullptr;

	// 위젯 숨김
	SetVisibility(ESlateVisibility::Collapsed);

	// 안내 텍스트 원래대로
	if (InstructionText)
	{
		InstructionText->SetText(FText::FromString(TEXT("E키를 눌러 구조")));
	}

	// 프로그레스 바 초기화
	if (ReviveProgressBar)
	{
		ReviveProgressBar->SetPercent(0.0f);
	}
}

void UGS_ReviveIndicatorWidget::UpdateProgress(float Progress)
{
	if (Progress > 0.0f)
	{
		// 진행도가 있으면 프로그레스 바 표시
		if (ReviveProgressBar)
		{
			ReviveProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
			ReviveProgressBar->SetPercent(Progress);
		}

		if (ProgressText)
		{
			ProgressText->SetVisibility(ESlateVisibility::HitTestInvisible);
			int32 ProgressPercent = FMath::RoundToInt(Progress * 100.0f);
			ProgressText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), ProgressPercent)));
		}

		// 진행도가 있으면 E키 안내 숨김
		if (InstructionText)
		{
			InstructionText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		// 진행도가 0이면 프로그레스 바 숨기고 E키 안내 표시
		if (ReviveProgressBar)
		{
			ReviveProgressBar->SetVisibility(ESlateVisibility::Collapsed);
			ReviveProgressBar->SetPercent(0.0f);
		}

		if (ProgressText)
		{
			ProgressText->SetVisibility(ESlateVisibility::Collapsed);
		}

		// E키 안내 다시 표시
		if (InstructionText)
		{
			InstructionText->SetText(FText::FromString(TEXT("E키를 눌러 구조")));
			InstructionText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void UGS_ReviveIndicatorWidget::ShowNearbyIndicator(AGS_Seeker* TargetSeeker)
{
	if (!IsValid(TargetSeeker))
	{
		return;
	}

	CurrentTarget = TargetSeeker;

	// 위젯 표시
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 대상 이름 표시
	if (TargetNameText)
	{
		if (AGS_PlayerState* PS = TargetSeeker->GetPlayerState<AGS_PlayerState>())
		{
			TargetNameText->SetText(FText::FromString(PS->GetPlayerName()));
		}
		else
		{
			TargetNameText->SetText(FText::FromString(TargetSeeker->GetName()));
		}
	}

	// 진행도 확인: 이미 진행도가 있으면 프로그레스 바 표시, 없으면 E키 안내 표시
	float CurrentProgress = TargetSeeker->GetReviveProgress();

	if (CurrentProgress > 0.0f)
	{
		// 진행도가 남아있음 - 프로그레스 바 표시 (진행도 감소 중)
		if (InstructionText)
		{
			InstructionText->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (ReviveProgressBar)
		{
			ReviveProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		if (ProgressText)
		{
			ProgressText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
	else
	{
		// 진행도 없음 - E키 안내만 표시
		if (InstructionText)
		{
			InstructionText->SetText(FText::FromString(TEXT("E키를 눌러 구조")));
			InstructionText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		if (ReviveProgressBar)
		{
			ReviveProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (ProgressText)
		{
			ProgressText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGS_ReviveIndicatorWidget::HideNearbyIndicator()
{
	CurrentTarget = nullptr;

	// 위젯 완전 숨김
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGS_ReviveIndicatorWidget::StopReviveProgress()
{
	bIsReviving = false;

	// E키 안내는 숨김 (진행도 감소 중에는 안내 텍스트 불필요)
	if (InstructionText)
	{
		InstructionText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 프로그레스 바는 명시적으로 표시 (진행도 감소를 보여주기 위해)
	if (ReviveProgressBar)
	{
		ReviveProgressBar->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (ProgressText)
	{
		ProgressText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

