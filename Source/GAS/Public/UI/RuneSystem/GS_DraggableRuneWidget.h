// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DraggableRuneWidget.generated.h"

class UButton;
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
	
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitRuneWidget(uint8 InRuneID, UTexture2D* InRuneTexture, UGS_ArcaneBoardWidget* BoardWidget=nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneID(uint8 InRuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	uint8 GetRuneID() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneTexture(UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetPlaced(bool bPlaced);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	uint8 RuneID;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	UTexture2D* RuneTexture;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* DragHandleButton;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	bool bIsPlaced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArcaneBoard")
	TSubclassOf<UGS_DragVisualWidget> DragVisualWidgetClass;

	UPROPERTY()
	UGS_ArcaneBoardWidget* ParentBoardWidget;
};
