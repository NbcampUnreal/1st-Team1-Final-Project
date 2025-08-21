// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_RuneInventoryWidget.h"
#include "UI/RuneSystem/GS_DraggableRuneWidget.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

UGS_RuneInventoryWidget::UGS_RuneInventoryWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
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

void UGS_RuneInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_RuneInventoryWidget::InitInven(UGS_ArcaneBoardManager* InBoardManager, UGS_ArcaneBoardWidget* InBoardWidget)
{
	BoardManager = InBoardManager;
	BoardWidget = InBoardWidget;

	if (!IsValid(BoardManager) || !IsValid(RuneScrollBox))
	{
		return;
	}

	RuneScrollBox->ClearChildren();
	OwnsRuneList.Empty();

	UGS_ArcaneBoardLPS* ArcaneBoardLPS = GetOwningLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>();
	if (!IsValid(ArcaneBoardLPS))
	{
		return;
	}

	TArray<uint8> OwnedRunes = ArcaneBoardLPS->GetOwnedRunes();

	for (uint8 RuneID : OwnedRunes)
	{
		FRuneTableRow RuneData;
		if (BoardManager->GetRuneData(RuneID, RuneData))
		{
			if (RuneWidgetClass)
			{
				UGS_DraggableRuneWidget* RuneWidget = CreateWidget<UGS_DraggableRuneWidget>(this, RuneWidgetClass);
				if (RuneWidget)
				{
					UTexture2D* RuneTexture = BoardManager->GetRuneTexture(RuneID);
					if (RuneTexture)
					{
						RuneWidget->InitRuneWidget(RuneID, RuneTexture, InBoardWidget);
					}

					for (const FPlacedRuneInfo& Rune : BoardManager->PlacedRunes)
					{
						if (Rune.RuneID == RuneID)
						{
							RuneWidget->SetPlaced(true);
						}
					}

					RuneScrollBox->AddChild(RuneWidget);
					OwnsRuneList.Add(RuneID, RuneWidget);
				}
			}
		}
	}

	int32 TotalRuneCnt = OwnsRuneList.Num();
	RuneCntText->SetText(FText::AsNumber(TotalRuneCnt));
}

bool UGS_RuneInventoryWidget::UpdatePlacedStateOfRune(uint8 RuneID, bool bIsPlaced)
{
	if (OwnsRuneList.Contains(RuneID))
	{
		OwnsRuneList[RuneID]->SetPlaced(bIsPlaced);
		return true;
	}

	return false;
}
