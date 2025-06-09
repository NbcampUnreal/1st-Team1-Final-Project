// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "GS_Timer.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_Timer : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TimerText;

	// 게임 총 시간 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings", meta = (ClampMin = "1.0"))
	float TotalGameTimeInSeconds = 900.0f; // 15분 = 900초

	// 경고 시간 임계값 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings", meta = (ClampMin = "1.0"))
	float WarningTimeThreshold = 300.0f; // 5분 = 300초

	// 위험 시간 임계값 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings", meta = (ClampMin = "1.0"))
	float CriticalTimeThreshold = 60.0f; // 1분 = 60초

	// 긴급 시간 임계값 (초 단위) - 흔들림 애니메이션 시작
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings", meta = (ClampMin = "1.0"))
	float UrgentTimeThreshold = 10.0f; // 10초

	// 기본 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Colors")
	FLinearColor NormalColor = FLinearColor::White;

	// 경고 색상 (5분 남았을 때)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Colors")
	FLinearColor WarningColor = FLinearColor::Yellow;

	// 위험 색상 (1분 남았을 때)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Colors")
	FLinearColor CriticalColor = FLinearColor::Red;

	// 긴급 색상 (10초 남았을 때)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Colors")
	FLinearColor UrgentColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f); // 더 밝은 빨간색

	// 흔들림 애니메이션 (블루프린트에서 설정)
	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* ShakeAnimation;

	// 애니메이션 강도 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ShakeIntensity = 1.0f;

private:
	UFUNCTION()
	void HandleTimeUpdated(const FText& NewTime);

	// 남은 시간에 따라 텍스트 색상을 업데이트
	void UpdateTextColor(float RemainingTimeInSeconds);

	// 흔들림 애니메이션 제어
	void StartShakeAnimation();
	void StopShakeAnimation();

	// 현재 애니메이션 상태 추적
	bool bIsShaking = false;
};
