// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_ChanAimingSkillBar.generated.h"

class UProgressBar;
class AGS_Character;

UCLASS()
class GAS_API UGS_ChanAimingSkillBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetOwningActor(AGS_Character* InOwningCharacter);

	// 일반적인 시간 소모(그냥 앞 바만 줄어듦)
	UFUNCTION(BlueprintCallable, Category="UI")
	void SetAimingProgress(float Progress);

	// 데미지로 줄어드는 경우(앞바 즉시, 뒤바 보간)
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetAimingProgressByDamage(float Progress);

	UFUNCTION(BlueprintCallable)
	void ShowSkillBar(bool bShow);

protected:
	// 앞바(즉시)
	UPROPERTY(meta=(BindWidget))
	UProgressBar* AimingProgressBar;

	// 뒤바(보간)
	UPROPERTY(meta = (BindWidget))
	UProgressBar* AimingProgressBar_back;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;

private:
	float TargetBackPercent = 1.0f;
	float BackBarInterpSpeed = 2.0f; // 보간 속도
};
