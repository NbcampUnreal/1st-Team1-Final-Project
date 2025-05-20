// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GS_RuneInventoryWidget.generated.h"

class UTextBlock;
class UScrollBox;
class UGS_DraggableRuneWidget;
class UGS_ArcaneBoardManager;
/**
 * 보유하고 있는 룬 목록을 표시하는 인벤토리 위젯
 */
UCLASS()
class GAS_API UGS_RuneInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGS_RuneInventoryWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitInven(UGS_ArcaneBoardManager* InBoardManager, UGS_ArcaneBoardWidget* InBoardWidget);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RuneCntText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* RuneScrollBox;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGS_DraggableRuneWidget> RuneWidgetClass;

	UPROPERTY()
	UGS_ArcaneBoardManager* BoardManager;
};
