// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RuneSystem/GS_ArcaneBoardTableRows.h"
#include "GS_RuneTooltipWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class GAS_API UGS_RuneTooltipWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UGS_RuneTooltipWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void SetRuneData(const FRuneTableRow& RuneData);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	uint8 GetRuneID();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RuneName;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RuneDescription;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RuneStat;

private:
	void UpdateTooltipUI();

	FRuneTableRow CurrRuneData;
};
