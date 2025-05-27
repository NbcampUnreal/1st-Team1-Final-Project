// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_StatDataWidget.generated.h"

class UTextBlock;
class UGS_ArcaneBoardManager;
/**
 * 
 */
UCLASS()
class GAS_API UGS_StatDataWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UGS_StatDataWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitStat(FName InStatName, float InBaseValue);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetAllStatData(FName InStatName, float InDefaultValue, float InRuneValue, float InBonusValue=0.0f);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneStatData(float InRuneValue, float InBonusValue = 0.0f);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatLabel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* DefaultValue;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RuneValue;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* BonusValue;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TotalValue;

private:
	FName StatName;
	float BaseValue;
	float CurrRuneValue;
	float CurrBonusValue;

	void UpdateUI();
};
