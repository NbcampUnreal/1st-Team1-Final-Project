// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_DragVisualWidget.generated.h"

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
	void Setup(uint8 InRuneID, UTexture2D* InTexture);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	uint8 RuneID;

	UPROPERTY(BlueprintReadWrite, Category = "ArcaneBoard")
	class UImage* RuneImage;
};
