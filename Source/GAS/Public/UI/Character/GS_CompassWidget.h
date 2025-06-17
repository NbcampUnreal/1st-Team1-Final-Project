// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "GS_CompassWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UImage;
class UGS_CompassIndicatorComponent;

USTRUCT(BlueprintType)
struct FCompassDirectionInfo
{
	GENERATED_BODY()

	FText Text;
	float Angle = 0.f;

	UPROPERTY()
	TObjectPtr<UTextBlock> Widget = nullptr;
};

USTRUCT(BlueprintType)
struct FTeammateIconInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UGS_CompassIndicatorComponent> IndicatorComponent;

	UPROPERTY()
	TObjectPtr<class UGS_TeammateIconWidget> IconWidget = nullptr;
};

UCLASS()
class GAS_API UGS_CompassWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	// The Canvas Panel where all compass elements (directions and icons) will be drawn.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CompassCanvasPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass|Appearance")
	FSlateFontInfo DirectionTextFont;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass|Appearance")
	TSubclassOf<UGS_TeammateIconWidget> TeammateIconClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass|Layout")
	float CompassWidth = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass|Layout")
	float DirectionSpacing = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compass|Behavior")
	float UpdateFrequency = 20.0f;

private:
	void InitializeDirections();
	void UpdateCompass();
	void UpdateCompassElements(float PlayerYaw);
	void RefreshTeammateIcons();

	float GetHorizontalPositionForAngle(float Angle, float PlayerYaw) const;
	
	UPROPERTY()
	TArray<FCompassDirectionInfo> Directions;
	
	UPROPERTY()
	TArray<FTeammateIconInfo> TeammateIcons;
	
	FTimerHandle RefreshIconsTimerHandle;
	FTimerHandle CompassUpdateTimerHandle;
}; 