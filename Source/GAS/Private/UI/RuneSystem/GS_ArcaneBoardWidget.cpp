// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "Components/UniformGridPanel.h"
#include "UI/RuneSystem/GS_RuneGridCellWidget.h"
#include "UI/RuneSystem/GS_RuneInventoryWidget.h"
#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"


UGS_ArcaneBoardWidget::UGS_ArcaneBoardWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SelectedRuneID = 0;
	bIsInSelectionMode = false;
	SelectionVisualWidget = nullptr;
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
	if (!IsValid(DragVisualWidgetClass))
	{
		FSoftClassPath Path(TEXT("/Game/UI/RuneSystem/WBP_DragVisual.WBP_DragVisual_C"));
		UClass* LoadedClass = Path.TryLoadClass<UGS_DragVisualWidget>();

		if (LoadedClass)
		{
			DragVisualWidgetClass = LoadedClass;
		}
	}

	//임시
	if (!IsValid(BoardManager))
	{
		BoardManager = NewObject<UGS_ArcaneBoardManager>(this);
		BoardManager->InitializeForTesting();
	}

	GenerateGridLayout();

	InitInventory();
}

FReply UGS_ArcaneBoardWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (!bIsInSelectionMode || !SelectionVisualWidget)
	{
		return Reply;
	}

	if(GetWorld())
	{
		FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

		SelectionVisualWidget->SetPositionInViewport(MousePos, false);

		UGS_RuneGridCellWidget* CellUnderMouse = GetCellAtPos(MousePos);

		if (CellUnderMouse)
		{
			UpdateGridPreview(SelectedRuneID, CellUnderMouse->GetCellPos());
		}
		else
		{
			ClearPreview();
		}
	}

	return Reply;
}

FReply UGS_ArcaneBoardWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	/*if (!bIsInSelectionMode)
	{
		return Reply;
	}*/

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
		UGS_RuneGridCellWidget* CellUnderMouse = GetCellAtPos(MousePos);

		if (CellUnderMouse)
		{
			EndRuneSelection(true);
			return FReply::Handled();
		}
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		EndRuneSelection(false);
		return FReply::Handled();
	}

	return Reply;
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
				UTexture2D* RuneTexture = CellData.RuneTextureFrag;
				if (RuneTexture)
				{
					CellWidget->SetRuneTexture(RuneTexture);
				}
			}
		}
	}
}

void UGS_ArcaneBoardWidget::InitInventory()
{
	if (IsValid(RuneInven) && IsValid(BoardManager))
	{
		RuneInven->InitInven(BoardManager, this);
	}
}

void UGS_ArcaneBoardWidget::UpdateStatsDisplay(const FCharacterStats& Stats)
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

void UGS_ArcaneBoardWidget::StartRuneSelection(uint8 RuneID)
{
	UE_LOG(LogTemp, Display, TEXT("룬 선택 시작: ID=%d"), RuneID);

	if (bIsInSelectionMode)
	{
		EndRuneSelection(false);
	}

	SelectedRuneID = RuneID;
	bIsInSelectionMode = true;

	if (IsValid(DragVisualWidgetClass))
	{
		SelectionVisualWidget = CreateWidget<UGS_DragVisualWidget>(this, DragVisualWidgetClass);
		if (SelectionVisualWidget)
		{
			UTexture2D* RuneTexture = nullptr;
			if (BoardManager)
			{
				RuneTexture = BoardManager->GetRuneTexture(RuneID);
			}

			SelectionVisualWidget->Setup(RuneID, RuneTexture);
			SelectionVisualWidget->AddToViewport(10000);

			if (GetWorld())
			{
				FVector2D MousePos = FVector2D::ZeroVector;
				MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
				if (MousePos != FVector2D::ZeroVector)
				{
					SelectionVisualWidget->SetPositionInViewport(MousePos, false);
				}
			}
		}
	}
}

void UGS_ArcaneBoardWidget::EndRuneSelection(bool bPlaceRune)
{
	if(!bPlaceRune)
	{
		if (SelectionVisualWidget)
		{
			SelectionVisualWidget->RemoveFromParent();
			SelectionVisualWidget = nullptr;
		}
	}
	else
	{
		//배치
	}

	bIsInSelectionMode = false;
	SelectedRuneID = 0;
}

UGS_RuneGridCellWidget* UGS_ArcaneBoardWidget::GetCellAtPos(const FVector2D& ScreenPosition)
{
	return nullptr;
}

void UGS_ArcaneBoardWidget::ClearPreview()
{
}

uint8 UGS_ArcaneBoardWidget::GetSelectedRuneID() const
{
	return SelectedRuneID;
}

void UGS_ArcaneBoardWidget::UpdateGridVisuals()
{
}
