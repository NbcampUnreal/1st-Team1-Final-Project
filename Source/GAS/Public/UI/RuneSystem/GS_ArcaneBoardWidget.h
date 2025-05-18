// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_ArcaneBoardWidget.generated.h"

class UUniformGridPanel;
class UGS_RuneInventoryWidget;
class UGS_StatPanelWidget;
class UGS_RuneGridCellWidget;
class UGS_ArcaneBoardManager;

/**
 * 아케인 보드 메인 위젯
 */
UCLASS()
class GAS_API UGS_ArcaneBoardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_ArcaneBoardWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void GenerateGridLayout();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitInventory();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateStatsDisplay(const FCharacterStats& Stats);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void OnRuneDragStarted(uint8 RuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void OnRuneDropped(uint8 RuneID, const FIntPoint& GridPosition);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void OnRuneDragCancelled();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateGridPreview(uint8 RuneID, const FIntPoint& GridPos);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void ApplyChanges();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void ResetBoard();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUniformGridPanel* GridPanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UGS_RuneInventoryWidget* RuneInven;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UGS_StatPanelWidget* StatPanel;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TArray<FIntPoint> PreviewCells;

	UPROPERTY(BlueprintReadWrite)
	UGS_ArcaneBoardManager* BoardManager;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGS_RuneGridCellWidget> GridCellWidgetClass;

	UPROPERTY()
	TMap<FIntPoint, UGS_RuneGridCellWidget*> GridCells;

	UPROPERTY(BlueprintReadWrite)
	uint8 CurrDragRuneID;
};
