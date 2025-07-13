// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DragVisualWidget.generated.h"

class UUniformGridPanel;
class UImage;
class USizeBox;

/**
 * 드래그 중 룬을 보여주는 위젯
 */
UCLASS()
class GAS_API UGS_DragVisualWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_DragVisualWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void Setup(uint8 InRuneID, UTexture2D* InTexture, const TMap<FIntPoint, UTexture2D*>& RuneShape, const FVector2D& InBaseCellSize, float ScaleFactor = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	FVector2D GetReferenceCellOffset() const;

protected:
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	uint8 RuneID;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* DragVisualSizeBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UUniformGridPanel* RuneGridPanel;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	TMap<FIntPoint, UTexture2D*> CachedRuneShape;

	UPROPERTY()
	TMap<FIntPoint, UImage*> GridCellWidgets;

	// 기준 셀 위치 (보통 0,0)
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	FIntPoint ReferenceCellPos;

	// 기준 셀의 위젯 내 오프셋 (픽셀 단위)
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	FVector2D ReferenceCellOffset;

private:
	UPROPERTY()
	TMap<FIntPoint, FIntPoint> CellToGridPosMap;

	UPROPERTY()
	FVector2D BaseCellSize;

	UPROPERTY()
	float CurrentScaleFactor;

	void CreateRuneShapeGrid(const TMap<FIntPoint, UTexture2D*>& RuneShape);

	void CalculateGridBounds(const TMap<FIntPoint, UTexture2D*>& RuneShape, FIntPoint& MinPos, FIntPoint& MaxPos);
	
	void CalculateInitReferenceCellOffset();
};
