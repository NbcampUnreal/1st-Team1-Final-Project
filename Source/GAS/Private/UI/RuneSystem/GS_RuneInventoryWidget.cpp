// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_RuneInventoryWidget.h"
#include "UI/RuneSystem/GS_DraggableRuneWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "Components/ScrollBox.h"

UGS_RuneInventoryWidget::UGS_RuneInventoryWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_RuneInventoryWidget::NativeConstruct()
{
	//임시
	if (!IsValid(RuneWidgetClass))
	{
		FSoftClassPath RuneWidgetClassPath(TEXT("/Game/UI/RuneSystem/WBP_DraggableRune.WBP_DraggableRune_C"));
		UClass* LoadedClass = RuneWidgetClassPath.TryLoadClass<UGS_DraggableRuneWidget>();

		if (LoadedClass)
		{
			RuneWidgetClass = LoadedClass;
		}
	}
}

void UGS_RuneInventoryWidget::InitInven(UGS_ArcaneBoardManager* InBoardManager, UGS_ArcaneBoardWidget* InBoardWidget)
{
	BoardManager = InBoardManager;

	if (!IsValid(BoardManager) || !IsValid(RuneScrollBox))
	{
		return;
	}

	RuneScrollBox->ClearChildren();

	//테스트 용
	for (uint8 RuneID=1; RuneID <=16; ++RuneID)
	{
		FRuneTableRow RuneData;
		if (BoardManager->GetRuneData(RuneID, RuneData))
		{
			UGS_DraggableRuneWidget* RuneWidget = CreateWidget<UGS_DraggableRuneWidget>(this, RuneWidgetClass);
			if (RuneWidget)
			{
				UTexture2D* RuneTexture = BoardManager->GetRuneTexture(RuneID);
				if (RuneTexture)
				{
					RuneWidget->InitRuneWidget(RuneID, RuneTexture, InBoardWidget);
				}

				RuneScrollBox->AddChild(RuneWidget);
			}
		}
	}
}
