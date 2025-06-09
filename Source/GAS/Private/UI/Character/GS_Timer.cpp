// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_Timer.h"
#include "Components/TextBlock.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GameState/GS_BossLevelGS.h"
#include "Engine/World.h"

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
		StartShakeAnimation();
	}
	else if (RemainingTimeInSeconds <= CriticalTimeThreshold)
	{
		// 1분 이하 - 빨간색 (흔들림 중지)
		TargetColor = CriticalColor;
		StopShakeAnimation();
	}
	else if (RemainingTimeInSeconds <= WarningTimeThreshold)
	{
		// 5분 이하 - 노란색
		TargetColor = WarningColor;
		StopShakeAnimation();
	}
	else
	{
		// 그 외 - 기본 색상 (흰색)
		TargetColor = NormalColor;
		StopShakeAnimation();
	}
	
	// 텍스트 색상 적용
	TimerText->SetColorAndOpacity(FSlateColor(TargetColor));
}

void UGS_Timer::StartShakeAnimation()
{
	if (!ShakeAnimation || bIsShaking) return;
	
	// 흔들림 애니메이션 무한 반복으로 재생
	PlayAnimation(ShakeAnimation, 0.0f, 0, EUMGSequencePlayMode::PingPong, ShakeIntensity);
	bIsShaking = true;
	
	UE_LOG(LogTemp, Warning, TEXT("Timer: 긴급! 흔들림 애니메이션 시작!"));
}

void UGS_Timer::StopShakeAnimation()
{
	if (!ShakeAnimation || !bIsShaking) return;
	
	// 애니메이션 중지
	StopAnimation(ShakeAnimation);
	bIsShaking = false;
	
	UE_LOG(LogTemp, Log, TEXT("Timer: 흔들림 애니메이션 중지"));
}
