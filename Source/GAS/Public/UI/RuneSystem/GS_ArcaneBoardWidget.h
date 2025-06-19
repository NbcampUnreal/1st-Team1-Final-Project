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
class UGS_DragVisualWidget;
class UGS_RuneTooltipWidget;
class UGS_ArcaneBoardManager;
class UGS_ArcaneBoardLPS;

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
	virtual void NativeDestruct() override;

	//마우스 이벤트
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	//기본 기능
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetBoardManager(UGS_ArcaneBoardManager* InBoardManager);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	UGS_ArcaneBoardManager* GetBoardManager() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void RefreshForCurrCharacter();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void GenerateGridLayout();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitInventory();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitStatPanel();

	UFUNCTION()
	void OnStatsChanged(const FArcaneBoardStats& NewStats);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateGridPreview(uint8 RuneID, const FIntPoint& GridPos);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void UpdateGridVisuals();

	//룬 선택 관련 함수
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void StartRuneSelection(uint8 RuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void EndRuneSelection(bool bPlaceRune = false);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	bool StartRuneReposition(uint8 RuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	UGS_RuneGridCellWidget* GetCellAtPos(const FVector2D& ViewportPos);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void ClearPreview();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	uint8 GetSelectedRuneID() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	bool HasUnsavedChanges() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void OnResetButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void OnApplyButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	FVector2D GetGridCellSize() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	FVector2D CalculateDragVisualScale(uint8 RuneID) const;

	//툴팁
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void RequestShowTooltip(uint8 RuneID, const FVector2D& MousePos);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void HideTooltip();

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void CancelTooltipRequest();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))

	class UButton* ApplyButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ResetButton;

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

	//툴팁
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArcaneBoard")
	TSubclassOf<UGS_RuneTooltipWidget> TooltipWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	UGS_RuneTooltipWidget* RuneTooltipWidget;

	FTimerHandle TooltipDelayTimer;

private:
	void BindToLPS();
	void UnbindFromLPS();

	UPROPERTY()
	UGS_ArcaneBoardLPS* ArcaneBoardLPS;

	//툴팁
	FVector2D TooltipSize;
	uint8 CurrTooltipRuneID;

	void ShowTooltip(uint8 RuneID, const FVector2D& MousePos);
	FVector2D CalculateTooltipPosition(const FVector2D& MousePos);
	bool ShouldShowTooltip() const;
};
