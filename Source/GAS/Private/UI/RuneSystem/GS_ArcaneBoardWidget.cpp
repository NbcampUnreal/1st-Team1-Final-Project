// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "UI/RuneSystem/GS_RuneGridCellWidget.h"
#include "UI/RuneSystem/GS_RuneInventoryWidget.h"
#include "UI/RuneSystem/GS_StatPanelWidget.h"
#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"


UGS_ArcaneBoardWidget::UGS_ArcaneBoardWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
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

	SelectedRuneID = 0;
	bIsInSelectionMode = false;
	SelectionVisualWidget = nullptr;
}

void UGS_ArcaneBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &UGS_ArcaneBoardWidget::OnApplyButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UGS_ArcaneBoardWidget::OnResetButtonClicked);
	}

	RefreshForCurrentCharacter();
}

void UGS_ArcaneBoardWidget::NativeDestruct()
{
	UnbindManagerEvents();

	if (IsValid(SelectionVisualWidget))
	{
		SelectionVisualWidget->RemoveFromParent();
		SelectionVisualWidget = nullptr;
	}

	Super::NativeDestruct();
}

void UGS_ArcaneBoardWidget::BindManagerEvents()
{
	if (IsValid(BoardManager))
	{
		UnbindManagerEvents();
		BoardManager->OnStatsChanged.AddDynamic(this, &UGS_ArcaneBoardWidget::OnStatsChanged);
	}
}

void UGS_ArcaneBoardWidget::UnbindManagerEvents()
{
	if (IsValid(BoardManager))
	{
		BoardManager->OnStatsChanged.RemoveDynamic(this, &UGS_ArcaneBoardWidget::OnStatsChanged);
	}
}

FReply UGS_ArcaneBoardWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (!bIsInSelectionMode || !SelectionVisualWidget)
	{
		return Reply;
	}

	// 스크린 좌표를 뷰포트 좌표로 변환
	FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
	if (APlayerController* PC = GetOwningPlayer())
	{
		FGeometry ScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(PC);
		MousePos = ScreenGeometry.AbsoluteToLocal(MousePos);
	}

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

	return Reply;
}

FReply UGS_ArcaneBoardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 스크린 좌표를 뷰포트 좌표로 변환
		FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
		if (APlayerController* PC = GetOwningPlayer())
		{
			FGeometry ScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(PC);
			MousePos = ScreenGeometry.AbsoluteToLocal(MousePos);
		}

		UGS_RuneGridCellWidget* CellUnderMouse = GetCellAtPos(MousePos);

		if (CellUnderMouse)
		{
			uint8 RuneID = CellUnderMouse->GetPlacedRuneID();

			//배치 로직
			if(bIsInSelectionMode)
			{
				EndRuneSelection(true);
				return FReply::Handled();
			}
			else if(RuneID > 0)
			{
				StartRuneReposition(RuneID);
				return FReply::Handled();
			}
		}
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		EndRuneSelection(false);
		return FReply::Handled();
	}

	return Reply;
}

void UGS_ArcaneBoardWidget::SetBoardManager(UGS_ArcaneBoardManager* InBoardManager)
{
	if (!InBoardManager)
	{
		return;
	}

	RefreshForCurrentCharacter();
}

UGS_ArcaneBoardManager* UGS_ArcaneBoardWidget::GetBoardManager() const
{
	return BoardManager;
}

void UGS_ArcaneBoardWidget::RefreshForCurrentCharacter()
{
	if (UGS_ArcaneBoardLPS* LPS = GetOwningLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>())
	{
		BoardManager = LPS->GetOrCreateBoardManager();

		if (IsValid(BoardManager))
		{
			BindManagerEvents();
			GenerateGridLayout();
			UpdateGridVisuals();
			InitInventory();
			InitStatPanel();
		}
	}
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
			CellWidget->InitCell(CellData, this);
			GridPanel->AddChildToUniformGrid(CellWidget, CellData.Pos.X, CellData.Pos.Y);
			GridCells.Add(CellData.Pos, CellWidget);
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

void UGS_ArcaneBoardWidget::InitStatPanel()
{
	if (IsValid(StatPanel) && IsValid(BoardManager))
	{
		StatPanel->InitStatList(BoardManager);

		OnStatsChanged(BoardManager->CurrStatEffects);
	}
}

void UGS_ArcaneBoardWidget::OnStatsChanged(const FGS_StatRow& NewStats)
{
	if (IsValid(StatPanel))
	{
		StatPanel->UpdateStats(NewStats);

		UE_LOG(LogTemp, Warning, TEXT("스탯 UI 업데이트 완료 - HP+%.1f, ATK+%.1f, DEF+%.1f, AGL+%.1f, ATS+%.1f"),
			NewStats.HP, NewStats.ATK, NewStats.DEF, NewStats.AGL, NewStats.ATS);
	}
}

void UGS_ArcaneBoardWidget::UpdateGridPreview(uint8 RuneID, const FIntPoint& GridPos)
{
	if (!IsValid(BoardManager))
	{
		return;
	}

	ClearPreview();

	TArray<FIntPoint> AffectedCells;
	bool bCanPlace = BoardManager->PreviewRunePlacement(RuneID, GridPos, AffectedCells);

	for (const FIntPoint& CellPos : AffectedCells)
	{
		if (GridCells.Contains(CellPos))
		{
			UGS_RuneGridCellWidget* CellWidget = GridCells[CellPos];
			if (IsValid(CellWidget))
			{
				EGridCellVisualState PreviewState = bCanPlace ?
					EGridCellVisualState::Valid : EGridCellVisualState::Invalid;

				CellWidget->SetPreviewVisualState(PreviewState);
			}
		}
	}

	PreviewCells = AffectedCells;
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

			FVector2D ScaleFactors = CalculateDragVisualScale(RuneID);
			SelectionVisualWidget->SetRenderScale(ScaleFactors);

			FVector2D OriginalSize = SelectionVisualWidget->GetDesiredSize();
			FVector2D ActualDragVisualSize = FVector2D(
				OriginalSize.X * ScaleFactors.X,
				OriginalSize.Y * ScaleFactors.Y
			);

			if (GetWorld())
			{
				FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
				if (MousePos != FVector2D::ZeroVector)
				{
					FVector2D CenterOffset = ActualDragVisualSize * 0.5f;
					SelectionVisualWidget->SetPositionInViewport(MousePos - CenterOffset, false);
				}
			}

			SelectionVisualWidget->AddToViewport(3);
		}
	}
}

void UGS_ArcaneBoardWidget::EndRuneSelection(bool bPlaceRune)
{
	if (!bPlaceRune)
	{
		if (IsValid(SelectionVisualWidget))
		{
			SelectionVisualWidget->RemoveFromParent();
			SelectionVisualWidget = nullptr;
		}
		ClearPreview();
	}
	else
	{
		if (IsValid(BoardManager) && SelectedRuneID > 0)
		{
			FVector2D MousePos = FVector2D::ZeroVector;
			if (GetWorld())
			{
				MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
			}

			UGS_RuneGridCellWidget* TargetCell = GetCellAtPos(MousePos);

			if (IsValid(TargetCell))
			{
				FIntPoint CellPos = TargetCell->GetCellPos();

				bool bPlaceSuccess = BoardManager->PlaceRune(SelectedRuneID, CellPos);

				if (bPlaceSuccess)
				{
					UE_LOG(LogTemp, Display, TEXT("룬 배치 성공: ID=%d, Pos=(%d,%d)"), SelectedRuneID, CellPos.X, CellPos.Y);
					UpdateGridVisuals();
					RuneInven->UpdatePlacedStateOfRune(SelectedRuneID, true);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("룬 배치 실패: ID=%d"), SelectedRuneID);
				}
			}

			if (IsValid(SelectionVisualWidget))
			{
				SelectionVisualWidget->RemoveFromParent();
				SelectionVisualWidget = nullptr;
			}
		}
		ClearPreview();
	}

	bIsInSelectionMode = false;
	SelectedRuneID = 0;
}

bool UGS_ArcaneBoardWidget::StartRuneReposition(uint8 RuneID)
{
	if (!BoardManager->RemoveRune(RuneID))
	{
		return false;
	}

	UpdateGridVisuals();
	RuneInven->UpdatePlacedStateOfRune(RuneID, false);
	StartRuneSelection(RuneID);

	return true;
}

UGS_RuneGridCellWidget* UGS_ArcaneBoardWidget::GetCellAtPos(const FVector2D& ViewportPos)
{
	// 뷰포트 좌표를 스크린 좌표로 변환
	FVector2D ScreenPos = ViewportPos;
	if (APlayerController* PC = GetOwningPlayer())
	{
		FGeometry ScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(PC);
		ScreenPos = ScreenGeometry.LocalToAbsolute(ViewportPos);
	}

	for (auto& CellPair : GridCells)
	{
		UGS_RuneGridCellWidget* CellWidget = CellPair.Value;
		FGeometry CellGeometry = CellWidget->GetCachedGeometry();
		FVector2D LocalMousePos = CellGeometry.AbsoluteToLocal(ScreenPos);

		FVector2D LocalSize = CellGeometry.GetLocalSize();
		if (LocalMousePos.X >= 0 && LocalMousePos.Y >= 0 &&
			LocalMousePos.X <= LocalSize.X && LocalMousePos.Y <= LocalSize.Y)
		{
			return CellWidget;
		}
	}
	return nullptr;
}

void UGS_ArcaneBoardWidget::ClearPreview()
{
	for (const FIntPoint& CellPos : PreviewCells)
	{
		if (GridCells.Contains(CellPos))
		{
			UGS_RuneGridCellWidget* CellWidget = GridCells[CellPos];
			if (IsValid(CellWidget))
			{
				CellWidget->SetPreviewVisualState(EGridCellVisualState::Normal);
			}
		}
	}

	PreviewCells.Empty();
}

uint8 UGS_ArcaneBoardWidget::GetSelectedRuneID() const
{
	return SelectedRuneID;
}

bool UGS_ArcaneBoardWidget::HasUnsavedChanges() const
{
	if (IsValid(BoardManager))
	{
		return BoardManager->bHasUnsavedChanges;
	}
	return false;
}

void UGS_ArcaneBoardWidget::OnResetButtonClicked()
{
	if (!IsValid(BoardManager))
	{
		return;
	}

	BoardManager->ResetAllRune();

	UpdateGridVisuals();
	RuneInven->InitInven(BoardManager, this);
}

void UGS_ArcaneBoardWidget::OnApplyButtonClicked()
{
	if (UGS_ArcaneBoardLPS* LPS = GetOwningLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>())
	{
		if (HasUnsavedChanges())
		{
			LPS->ApplyBoardChanges();
		}
	}
}

FVector2D UGS_ArcaneBoardWidget::GetGridCellSize() const
{
	if (GridCells.Num() == 0 || !IsValid(GridPanel))
	{
		return FVector2D::ZeroVector;
	}

	for (const auto& CellPair : GridCells)
	{
		if (IsValid(CellPair.Value))
		{
			FGeometry CellGeometry = CellPair.Value->GetCachedGeometry();
			return CellGeometry.GetLocalSize();
		}
	}

	return FVector2D::ZeroVector;
}

FVector2D UGS_ArcaneBoardWidget::CalculateDragVisualScale(uint8 RuneID) const
{
	if (!IsValid(BoardManager))
	{
		return FVector2D(1.0f, 1.0f);
	}

	FRuneTableRow RuneData;
	if (!BoardManager->GetRuneData(RuneID, RuneData))
	{
		return FVector2D(1.0f, 1.0f);
	}

	FVector2D CellSize = GetGridCellSize();
	if (CellSize.IsZero())
	{
		return FVector2D(1.0f, 1.0f);
	}

	FIntPoint RuneSize = RuneData.RuneSize;

	FVector2D TargetSize = FVector2D(
		CellSize.X * RuneSize.Y,
		CellSize.Y * RuneSize.X
	);

	float BaseSize = 64.0f;

	float ScaleX = TargetSize.X / BaseSize;
	float ScaleY = TargetSize.Y / BaseSize;

	return FVector2D(ScaleX * 0.8f, ScaleY * 0.8f);
}

void UGS_ArcaneBoardWidget::UpdateGridVisuals()
{
	if (!IsValid(BoardManager))
	{
		return;
	}

	for (auto Cell : GridCells)
	{
		UGS_RuneGridCellWidget* CellWidget = Cell.Value;
		FGridCellData UpdatedCellData;

		if (BoardManager->GetCellData(Cell.Key, UpdatedCellData))
		{
			CellWidget->SetCellData(UpdatedCellData);
		}
	}
}