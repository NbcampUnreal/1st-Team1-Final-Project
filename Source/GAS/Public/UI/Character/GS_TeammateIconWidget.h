// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_TeammateIconWidget.generated.h"

class UImage;
class UGS_CompassIndicatorComponent;

UCLASS()
class GAS_API UGS_TeammateIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Teammate Icon")
	void SetIconAppearance(UGS_CompassIndicatorComponent* Indicator);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> IconImage;
}; 