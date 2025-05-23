// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/GS_ArcaneBoardTypes.h"
#include "Blueprint/UserWidget.h"
#include "GS_ArcaneBoardWidget.generated.h"

class UUniformGridPanel;
class UGS_RuneInventoryWidget;
class UGS_StatPanelWidget;
class UGS_RuneGridCellWidget;
class UGS_ArcaneBoardManager;
class UGS_DragVisualWidget;

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

	//마우스 이벤트
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	//기본 기능
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void GenerateGridLayout();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitInventory();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateStatsDisplay(const FCharacterStats& Stats);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateGridPreview(uint8 RuneID, const FIntPoint& GridPos);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void ApplyChanges();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void ResetBoard();

	//룬 선택 관련 함수
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void StartRuneSelection(uint8 RuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void EndRuneSelection(bool bPlaceRune = false);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	UGS_RuneGridCellWidget* GetCellAtPos(const FVector2D& ViewportPos);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void ClearPreview();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	uint8 GetSelectedRuneID() const;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArcaneBoard")
	TSubclassOf<UGS_DragVisualWidget> DragVisualWidgetClass;

	UPROPERTY()
	TMap<FIntPoint, UGS_RuneGridCellWidget*> GridCells;

	UPROPERTY(BlueprintReadWrite)
	uint8 SelectedRuneID;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	bool bIsInSelectionMode;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	UGS_DragVisualWidget* SelectionVisualWidget;

private:
	float DragVisualOffset;

	void UpdateGridVisuals();

	FVector2D ScreenToViewport(const FVector2D& ScreenPos) const;
	FVector2D ViewportToScreen(const FVector2D& ViewportPos) const;
};
