// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Component/GS_StatRow.h"
#include "GS_StatPanelWidget.generated.h"

class UVerticalBox;
class UGS_ArcaneBoardManager;
class UGS_StatDataWidget;

/**
 * 
 */
UCLASS()
class GAS_API UGS_StatPanelWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UGS_StatPanelWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitStatList(UGS_ArcaneBoardManager* InBoardManager);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateStats(const FArcaneBoardStats& RuneStats);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	float GetStatValueByName(const FGS_StatRow& StatRow, FName StatName);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* StatDatas;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGS_StatDataWidget> StatWidgetClass;

	UPROPERTY()
	UGS_ArcaneBoardManager* BoardManager;

	UPROPERTY()
	FGS_StatRow BaseStatRow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ArcaneBoard")
	TObjectPtr<UDataTable> StatDataTable;

	UPROPERTY()
	TMap<FName, UGS_StatDataWidget*> StatDataList;
};
