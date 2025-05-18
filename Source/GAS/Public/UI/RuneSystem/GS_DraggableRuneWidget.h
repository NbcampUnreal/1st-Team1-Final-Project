// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DraggableRuneWidget.generated.h"

class UButton;

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

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneID(uint8 InRuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	uint8 GetRuneID() const;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void SetRuneTexture(UTexture2D* Texture);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	uint8 RuneID;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* DragHandleButton;
};
