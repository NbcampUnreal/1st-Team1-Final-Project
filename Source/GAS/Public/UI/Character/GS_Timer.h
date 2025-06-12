// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "GS_Timer.generated.h"

class UTextBlock;
class AGS_InGameGS;
class AGS_BossLevelGS;

UCLASS()
class GAS_API UGS_Timer : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;

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

	// 흔들림 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float ShakeIntensity = 3.0f; // 흔들림 강도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ShakeSpeed = 0.8f; // 흔들림 속도

private:
	UPROPERTY()
	TWeakObjectPtr<AGS_InGameGS> CachedInGameGS;
	UPROPERTY()
	TWeakObjectPtr<AGS_BossLevelGS> CachedBossLevelGS;

	FTimerHandle InitTimerHandle;

	int32 LastDisplayedSeconds = -1;

	void TryInitialize();

	// 남은 시간에 따라 텍스트 색상을 업데이트
	void UpdateTextColor(float RemainingTimeInSeconds);

	// 흔들림 효과
	void StartShakeEffect();
	void StopShakeEffect();
	void UpdateShakeEffect();

	// 흔들림 상태 추적
	bool bIsShaking = false;
	float ShakeTimer = 0.0f;
	FVector2D OriginalPosition = FVector2D::ZeroVector;
	bool bOriginalPositionSaved = false;

	// 타이머 핸들
	FTimerHandle ShakeTimerHandle;
};
