// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DyingStateWidget.generated.h"

class UProgressBar;
class UTextBlock;
class AGS_Seeker;

/**
 * 빈사 상태 UI 위젯
 * - 남은 빈사 시간 타이머 표시
 * - 구조 진행도 바 표시
 */
UCLASS()
class GAS_API UGS_DyingStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	/** 빈사 상태 시커 설정 */
	UFUNCTION(BlueprintCallable, Category = "Dying")
	void SetDyingSeeker(AGS_Seeker* InSeeker);

	/** 위젯 표시/숨김 */
	UFUNCTION(BlueprintCallable, Category = "Dying")
	void ShowWidget();

	UFUNCTION(BlueprintCallable, Category = "Dying")
	void HideWidget();

protected:
	// ==========================================
	// 빈사 타이머 UI
	// ==========================================
	
	/** 남은 시간 프로그레스 바 (원형 또는 직선) */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* DyingTimerBar;

	/** 남은 시간 텍스트 (초 단위) */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DyingTimerText;

	// ==========================================
	// 구조 진행도 UI
	// ==========================================

	/** 구조 진행도 바 */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* ReviveProgressBar;

	/** 구조 중 텍스트 (예: "구조 중...") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ReviveStatusText;

	// ==========================================
	// 설정
	// ==========================================

	/** 위험 상태 임계값 (초) - 이 시간 이하면 색상 변경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "5.0", ClampMax = "60.0"))
	float CriticalTimeThreshold = 30.0f;

	/** 일반 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NormalColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);  // 주황색

	/** 위험 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor CriticalColor = FLinearColor::Red;

	/** 구조 진행 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ReviveColor = FLinearColor::Green;

private:
	/** 연결된 빈사 상태 시커 */
	UPROPERTY()
	TWeakObjectPtr<AGS_Seeker> DyingSeeker;

	/** 빈사 상태 변화 델리게이트 핸들러 */
	UFUNCTION()
	void OnDyingStateChanged(bool bIsDying, float TimeRemaining);

	/** 구조 진행도 변화 델리게이트 핸들러 */
	UFUNCTION()
	void OnReviveProgressChanged(float Progress);

	/** UI 업데이트 */
	void UpdateTimerUI(float TimeRemaining, float MaxTime);
	void UpdateReviveUI(float Progress, bool bIsBeingRevived);

	/** 델리게이트 바인딩/해제 */
	void BindDelegates();
	void UnbindDelegates();
};

