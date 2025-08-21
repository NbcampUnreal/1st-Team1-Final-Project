#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h" 
#include "GS_HPWidget.generated.h"


class UGS_StatComp;
class AGS_Character;
class UProgressBar;
class UImage;
class USlider;
class UGS_WidgetShakeHelper;

UCLASS()
class GAS_API UGS_HPWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_HPWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void InitializeHPWidget(UGS_StatComp* InStatComp);
	
	AGS_Character* GetOwningActor()const { return OwningCharacter; }

	void SetOwningActor(AGS_Character* InOwningCharacter) { OwningCharacter = InOwningCharacter; }

	UFUNCTION()
	void OnCurrentHPBarChanged(UGS_StatComp* InStatComp);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr <UProgressBar> HPBarWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr <UProgressBar> HPDelayBarWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> HPSlider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGS_Character> OwningCharacter;

	float MaxHealth;
	float TargetHPPercent = 1.f;
	float CurrentHPPercent = 1.f;
	float DelayedHPPercent = 1.f;

	float InterpSpeed = 0.5f;

	FTimerHandle DelayBeforInterpTimerHandle;
	FTimerHandle InterpTimerHandle;

	// 흔들림 헬퍼 (UObject 기반)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_WidgetShakeHelper> ShakeHelper;

private:
	void UpdateDelayedHP();
	void UpdateMainHP();
	void StartDelayBarInterp();
};