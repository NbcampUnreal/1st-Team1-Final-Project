// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "Components/UniformGridPanel.h"
#include "UI/RuneSystem/GS_RuneGridCellWidget.h"

UGS_ArcaneBoardWidget::UGS_ArcaneBoardWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	CurrDragRuneID = 0;
}

void UGS_ArcaneBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//임시
	if (!IsValid(GridCellWidgetClass))
	{
		FSoftClassPath Path(TEXT("/Game/UI/RuneSystem/WBP_GridCell.WBP_GridCell_C"));
		UClass* LoadedClass = Path.TryLoadClass<UGS_RuneGridCellWidget>();

		if (LoadedClass)
		{
			GridCellWidgetClass = LoadedClass;
		}
	}

	//임시
	if (!IsValid(BoardManager))
	{
		BoardManager = NewObject<UGS_ArcaneBoardManager>(this);
		BoardManager->InitializeForTesting();
	}

	GenerateGridLayout();
}

void UGS_ArcaneBoardWidget::GenerateGridLayout()
{
	if (!IsValid(BoardManager) || !IsValid(GridPanel) || !IsValid(GridCellWidgetClass))
	{
		return;
	}

	UGS_GridLayoutDataAsset* GridLayout = BoardManager->GetCurrGridLayout();
	if (!GridLayout)
	{
		return;
	}

	GridPanel->ClearChildren();
	GridCells.Empty();

	for (const FGridCellData& CellData : GridLayout->GridCells)
	{
		UGS_RuneGridCellWidget* CellWidget = CreateWidget<UGS_RuneGridCellWidget>(this, GridCellWidgetClass);
		if (CellWidget)
		{
			CellWidget->SetCellData(CellData);
			GridPanel->AddChildToUniformGrid(CellWidget, CellData.Pos.X, CellData.Pos.Y);
			GridCells.Add(CellData.Pos, CellWidget);

			if (CellData.State == EGridCellState::Occupied && CellData.PlacedRuneID > 0)
			{
				UTexture2D* RuneTexture = BoardManager->GetRuneTexture(CellData.PlacedRuneID);
				if (RuneTexture)
				{
					CellWidget->SetRuneTexture(RuneTexture);
				}
			}
		}
	}


}

void UGS_ArcaneBoardWidget::UpdateStatsDisplay(const FCharacterStats& Stats)
{
}

void UGS_ArcaneBoardWidget::OnRuneDragStarted(uint8 RuneID)
{
}

void UGS_ArcaneBoardWidget::OnRuneDropped(uint8 RuneID, const FIntPoint& GridPosition)
{
}

void UGS_ArcaneBoardWidget::OnRuneDragCancelled()
{
}

void UGS_ArcaneBoardWidget::UpdateGridPreview(uint8 RuneID, const FIntPoint& GridPos)
{
}

void UGS_ArcaneBoardWidget::ApplyChanges()
{
}

void UGS_ArcaneBoardWidget::ResetBoard()
{
}
