// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GS_RuneDragOperation.generated.h"

class UGS_DraggableRuneWidget;
class UUserWidget;

/**
 * 룬 드래그 작업을 처리하는 클래스
 */
UCLASS()
class GAS_API UGS_RuneDragOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	uint8 RuneID;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	FVector2D DragOffset;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitDragOp(uint8 InRuneID, UUserWidget* InDragVisual);
};
