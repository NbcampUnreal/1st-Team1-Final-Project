// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_Timer.h"
#include "Components/TextBlock.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GameState/GS_BossLevelGS.h"
#include "Engine/World.h"
#include "Components/CanvasPanelSlot.h"

void UGS_Timer::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TimerText) return;
	
	// 기본 색상으로 초기화
	TimerText->SetColorAndOpacity(FSlateColor(NormalColor));
	
	if (AGS_InGameGS* GS = GetWorld()->GetGameState<AGS_InGameGS>())
	{
		// 델리게이트에 바인딩
		GS->OnTimerUpdated.AddDynamic(this, &UGS_Timer::HandleTimeUpdated);
		TimerText->SetText(GS->GetFormattedTime());
		
		// 초기 색상 설정 - TotalGameTime과 CurrentTime으로 계산
		float RemainingTime = FMath::Max(0.0f, GS->TotalGameTime - GS->CurrentTime);
		UpdateTextColor(RemainingTime);
	}

	if (AGS_BossLevelGS* BossGS = GetWorld()->GetGameState<AGS_BossLevelGS>())
	{
		BossGS->OnBossTimerUpdatedDelegate.AddDynamic(this, &UGS_Timer::HandleTimeUpdated);
		TimerText->SetText(BossGS->GetFormattedBossTime());
		
		// 보스 레벨의 경우 남은 시간을 계산해서 색상 업데이트
		float RemainingTime = FMath::Max(0.0f, BossGS->BossTotalTime - BossGS->BossCurrentTime);
		UpdateTextColor(RemainingTime);
	}
}

void UGS_Timer::HandleTimeUpdated(const FText& NewTime)
{
	if (!TimerText) return;
	
	TimerText->SetText(NewTime);
	
	// GameState에서 남은 시간을 가져와서 색상 업데이트
	float RemainingTime = 0.0f;
	
	if (AGS_InGameGS* GS = GetWorld()->GetGameState<AGS_InGameGS>())
	{
		RemainingTime = FMath::Max(0.0f, GS->TotalGameTime - GS->CurrentTime);
	}
	else if (AGS_BossLevelGS* BossGS = GetWorld()->GetGameState<AGS_BossLevelGS>())
	{
		RemainingTime = FMath::Max(0.0f, BossGS->BossTotalTime - BossGS->BossCurrentTime);
	}
	
	UpdateTextColor(RemainingTime);
}

void UGS_Timer::UpdateTextColor(float RemainingTimeInSeconds)
{
	if (!TimerText) return;
	
	FLinearColor TargetColor = NormalColor;
	
	// 남은 시간에 따라 색상 결정
	if (RemainingTimeInSeconds <= UrgentTimeThreshold)
	{
		// 10초 이하 - 긴급 상황 (밝은 빨간색 + 흔들림)
		TargetColor = UrgentColor;
		StartShakeEffect();
	}
	else if (RemainingTimeInSeconds <= CriticalTimeThreshold)
	{
		// 1분 이하 - 빨간색 (흔들림 중지)
		TargetColor = CriticalColor;
		StopShakeEffect();
	}
	else if (RemainingTimeInSeconds <= WarningTimeThreshold)
	{
		// 5분 이하 - 노란색
		TargetColor = WarningColor;
		StopShakeEffect();
	}
	else
	{
		// 그 외 - 기본 색상 (흰색)
		TargetColor = NormalColor;
		StopShakeEffect();
	}
	
	// 텍스트 색상 적용
	TimerText->SetColorAndOpacity(FSlateColor(TargetColor));
}

void UGS_Timer::StartShakeEffect()
{
	if (bIsShaking) return;
	
	bIsShaking = true;
	ShakeTimer = 0.0f;
	
	// 원본 위치 저장
	if (!bOriginalPositionSaved)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerText->Slot))
		{
			OriginalPosition = CanvasSlot->GetPosition();
			bOriginalPositionSaved = true;
		}
	}
	
	// 흔들림 타이머 시작 (60fps로 부드러운 애니메이션)
	GetWorld()->GetTimerManager().SetTimer(ShakeTimerHandle, this, &UGS_Timer::UpdateShakeEffect, 1.0f / 60.0f, true);
	
	UE_LOG(LogTemp, Warning, TEXT("Timer: 흔들림 효과 시작!"));
}

void UGS_Timer::StopShakeEffect()
{
	if (!bIsShaking) return;
	
	bIsShaking = false;
	
	// 타이머 중지
	GetWorld()->GetTimerManager().ClearTimer(ShakeTimerHandle);
	
	// 원본 위치로 복원
	if (bOriginalPositionSaved)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerText->Slot))
		{
			CanvasSlot->SetPosition(OriginalPosition);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("Timer: 흔들림 효과 중지"));
}

void UGS_Timer::UpdateShakeEffect()
{
	if (!bIsShaking || !TimerText) return;
	
	ShakeTimer += 1.0f / 60.0f; // 델타타임 추가
	
	// 사인파를 이용한 좌우 흔들림 계산
	float ShakeOffsetX = FMath::Sin(ShakeTimer * ShakeSpeed * 10.0f) * ShakeIntensity;
	float ShakeOffsetY = FMath::Sin(ShakeTimer * ShakeSpeed * 12.0f) * (ShakeIntensity * 0.5f); // Y축은 더 부드럽게
	
	// 위치 적용
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(TimerText->Slot))
	{
		FVector2D NewPosition = OriginalPosition + FVector2D(ShakeOffsetX, ShakeOffsetY);
		CanvasSlot->SetPosition(NewPosition);
	}
}
