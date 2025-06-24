// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "GS_CrossHairImage.generated.h"

class UImage;
class UWidgetSwitcher;
class AGS_Character;
class UGS_ArrowCounter;

UCLASS()
class GAS_API UGS_CrossHairImage : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* Aim_Anim;

	// 명중 피드백 애니메이션
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* HitFeedback_Anim;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Center;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* ArrowCounterSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGS_ArrowCounter> AxeArrowCounter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGS_ArrowCounter> ChildArrowCounter;

	void SetOwingActor(AGS_Character* InOwningCharacter);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void PlayAimAnim(bool bIsAiming);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void PlayHitFeedback();

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	bool IsAiming() const { return bCurrentlyAiming; }

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void UpdateArrowType(EArrowType NewType);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void UpdateArrowCnt(EArrowType ArrowType, int32 CurrCnt);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void InitArrowCounters();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;
	
private:
	bool bCurrentlyAiming = false;

	bool bIsAnimating = false;

	FWidgetAnimationDynamicEvent AimAnimEndDelegate;

	UFUNCTION()
	void OnAimAnimFinished();
};
