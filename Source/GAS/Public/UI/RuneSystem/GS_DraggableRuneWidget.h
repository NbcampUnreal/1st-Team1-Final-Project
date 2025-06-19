// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DraggableRuneWidget.generated.h"

class UImage;
class UGS_ArcaneBoardWidget;
class UGS_DragVisualWidget;

/**
 * 드래그 가능한 룬 위젯
 */
UCLASS()
class GAS_API UGS_DraggableRuneWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGS_DraggableRuneWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitRuneWidget(uint8 InRuneID, UTexture2D* InRuneTexture, UGS_ArcaneBoardWidget* BoardWidget=nullptr);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	uint8 GetRuneID() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneTexture(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneVisualState(bool bHovered, bool bDisabled = false);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetPlaced(bool bPlaced);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	uint8 RuneID;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* RuneImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* RuneState;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* SelectionIndicator;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	bool bIsPlaced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArcaneBoard")
	TSubclassOf<UGS_DragVisualWidget> DragVisualWidgetClass;

	UPROPERTY()
	UGS_ArcaneBoardWidget* ParentBoardWidget;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	bool bIsCurrentlyHovered;
};
