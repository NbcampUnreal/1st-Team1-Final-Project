// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_ReviveIndicatorWidget.generated.h"

class UProgressBar;
class UTextBlock;
class AGS_Seeker;

/**
 * 구조 표시 위젯 (구조하는 플레이어에게 표시)
 * - 구조 진행도 바
 * - 대상 이름 표시
 */
UCLASS()
class GAS_API UGS_ReviveIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 구조 시작 표시 */
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void StartRevive(AGS_Seeker* TargetSeeker);

	/** 구조 중지 표시 */
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void StopRevive();

	/** 구조 진행도 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void UpdateProgress(float Progress);

	/** 근처 빈사 시커 감지 시 표시 */
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void ShowNearbyIndicator(AGS_Seeker* TargetSeeker);

	/** 근처 빈사 시커 없을 때 숨김 */
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void HideNearbyIndicator();

	/** 구조 진행도만 중지 (위젯은 유지) */
	UFUNCTION(BlueprintCallable, Category = "Revive")
	void StopReviveProgress();

protected:
	/** 구조 진행도 바 */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* ReviveProgressBar;

	/** 안내 텍스트 (예: "E키를 눌러 구조") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* InstructionText;

	/** 대상 이름 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TargetNameText;

	/** 진행도 퍼센트 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ProgressText;

	/** 구조 진행 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ReviveColor = FLinearColor::Green;

private:
	/** 현재 구조 대상 */
	UPROPERTY()
	TWeakObjectPtr<AGS_Seeker> CurrentTarget;

	/** 구조 중인지 여부 */
	bool bIsReviving = false;
};

