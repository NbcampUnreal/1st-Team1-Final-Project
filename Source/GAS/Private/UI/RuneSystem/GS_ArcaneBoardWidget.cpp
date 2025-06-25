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
#include "UI/RuneSystem/GS_RuneTooltipWidget.h"
#include "Kismet/GameplayStatics.h"


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

	//임시
	if (!IsValid(TooltipWidgetClass))
	{
		FSoftClassPath Path(TEXT("/Game/UI/RuneSystem/WBP_RuneTooltip.WBP_RuneTooltip_C"));
		UClass* LoadedClass = Path.TryLoadClass<UGS_RuneTooltipWidget>();

		if (LoadedClass)
		{
			TooltipWidgetClass = LoadedClass;
		}
	}

	SelectedRuneID = 0;
	bIsInSelectionMode = false;
	SelectionVisualWidget = nullptr;
	RuneTooltipWidget = nullptr;
	TooltipSize = FVector2D(100.f, 100.f);
	CurrTooltipRuneID = 0;
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

	BindToLPS();
}

void UGS_ArcaneBoardWidget::NativeDestruct()
{
	UnbindFromLPS();

	if (IsValid(SelectionVisualWidget))
	{
		SelectionVisualWidget->RemoveFromParent();
		SelectionVisualWidget = nullptr;
	}

	if (IsValid(RuneTooltipWidget))
	{
		RuneTooltipWidget->RemoveFromParent();
		RuneTooltipWidget = nullptr;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TooltipDelayTimer);
	}

	Super::NativeDestruct();
}

FReply UGS_ArcaneBoardWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	// 스크린 좌표를 뷰포트 좌표로 변환
	FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
	if (APlayerController* PC = GetOwningPlayer())
	{
		FGeometry ScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(PC);
		MousePos = ScreenGeometry.AbsoluteToLocal(MousePos);
	}

	if (RuneTooltipWidget)
	{
		FVector2D TooltipPos = CalculateTooltipPosition(MousePos);
		RuneTooltipWidget->SetPositionInViewport(TooltipPos, false);
	}

	if (!bIsInSelectionMode || !SelectionVisualWidget)
	{
		return Reply;
	}

	//임시
	FVector2D AdditionalOffset = GetRuneDragOffset();

	SelectionVisualWidget->SetPositionInViewport(MousePos - AdditionalOffset, false);

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
	
	HideTooltip();

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
		// 우클릭으로 취소할 때는 EndRuneSelection에서 취소 사운드를 재생
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

	RefreshForCurrCharacter();
}

UGS_ArcaneBoardManager* UGS_ArcaneBoardWidget::GetBoardManager() const
{
	return BoardManager;
}

void UGS_ArcaneBoardWidget::RefreshForCurrCharacter()
{
	if (IsValid(BoardManager))
	{
		GenerateGridLayout();
		UpdateGridVisuals();
		InitInventory();
		InitStatPanel();
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

		OnStatsChanged(BoardManager->CurrBoardStats);
	}
}

void UGS_ArcaneBoardWidget::OnStatsChanged(const FArcaneBoardStats& NewStats)
{
	if (IsValid(StatPanel))
	{
		StatPanel->UpdateStats(NewStats);
	}
}

void UGS_ArcaneBoardWidget::UpdateGridPreview(uint8 RuneID, const FIntPoint& GridPos)
{
	if (!IsValid(BoardManager))
	{
		return;
	}

	ClearPreview();

	FIntPoint OptimalPos;
	if (BoardManager->FindOptimalPlacementPos(RuneID, GridPos, OptimalPos))
	{
		TArray<FIntPoint> AffectedCells;
		bool bCanPlace = BoardManager->PreviewRunePlacement(RuneID, OptimalPos, AffectedCells);

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
	else
	{
		TArray<FIntPoint> AffectedCells;
		bool bCanPlace = BoardManager->PreviewRunePlacement(RuneID, GridPos, AffectedCells);

		for (const FIntPoint& CellPos : AffectedCells)
		{
			if (GridCells.Contains(CellPos))
			{
				UGS_RuneGridCellWidget* CellWidget = GridCells[CellPos];
				if (IsValid(CellWidget))
				{
					CellWidget->SetPreviewVisualState(EGridCellVisualState::Invalid);
				}
			}
		}
		PreviewCells = AffectedCells;
	}
}

void UGS_ArcaneBoardWidget::StartRuneSelection(uint8 RuneID)
{
	UE_LOG(LogTemp, Display, TEXT("룬 선택 시작: ID=%d"), RuneID);

	// 룬 픽업 사운드 재생
	if (RunePickupSound)
	{
		UGameplayStatics::PlaySound2D(this, RunePickupSound);
	}

	HideTooltip();

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

			if (GetWorld())
			{
				FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
				if (MousePos != FVector2D::ZeroVector)
				{
					//임시
					FVector2D AdditionalOffset = GetRuneDragOffset();

					SelectionVisualWidget->SetPositionInViewport(MousePos - AdditionalOffset, false);
				}
			}

			SelectionVisualWidget->AddToViewport(3);
		}
	}

	// 룬 픽업 사운드 재생
	UGameplayStatics::PlaySound2D(this, RunePickupSound);
}

void UGS_ArcaneBoardWidget::EndRuneSelection(bool bPlaceRune)
{
	HideTooltip();

	if (!bPlaceRune)
	{
		// 룬 취소 사운드 재생
		if (RuneCancelSound)
		{
			UGameplayStatics::PlaySound2D(this, RuneCancelSound);
		}

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
				FIntPoint ClickedCellPos = TargetCell->GetCellPos();
				FIntPoint OptimalPos;

				if (BoardManager->FindOptimalPlacementPos(SelectedRuneID, ClickedCellPos, OptimalPos))
				{
					// 연결 상태 미리 체크하여 보너스 사운드 여부 결정
					bool bHadConnectionBefore = false;
					int32 PreviousConnectedRuneCnt = 0;
					if (BoardManager)
					{
						PreviousConnectedRuneCnt = BoardManager->ConnectedRuneCnt;
					}

					bool bPlaceSuccess = BoardManager->PlaceRune(SelectedRuneID, OptimalPos);

					if (bPlaceSuccess)
					{
						// 룬 배치 성공 사운드
						if (RunePlaceSuccessSound)
						{
							UGameplayStatics::PlaySound2D(this, RunePlaceSuccessSound);
						}

						// 연결 보너스 체크 및 사운드 재생
						if (BoardManager && BoardManager->ConnectedRuneCnt > PreviousConnectedRuneCnt)
						{
							if (RuneConnectionBonusSound)
							{
								// 약간의 딜레이 후 연결 보너스 사운드 재생
								if (GetWorld())
								{
									FTimerHandle ConnectionSoundTimer;
									GetWorld()->GetTimerManager().SetTimer(ConnectionSoundTimer, [this]()
									{
										UGameplayStatics::PlaySound2D(this, RuneConnectionBonusSound);
									}, 0.3f, false);
								}
							}
						}

						UpdateGridVisuals();
						RuneInven->UpdatePlacedStateOfRune(SelectedRuneID, true);
					}
					else
					{
						// 룸 배치 실패 사운드
						if (RunePlaceFailSound)
						{
							UGameplayStatics::PlaySound2D(this, RunePlaceFailSound);
						}
						UE_LOG(LogTemp, Warning, TEXT("룬 배치 실패: ID=%d"), SelectedRuneID);
					}
				}
				else
				{
					// 룬 배치 실패 사운드
					if (RunePlaceFailSound)
					{
						UGameplayStatics::PlaySound2D(this, RunePlaceFailSound);
					}
					UE_LOG(LogTemp, Warning, TEXT("배치 위치를 찾을 수 없음: ID=%d"), SelectedRuneID);
				}
			}
			else
			{
				// 잘못된 위치 클릭 시 실패 사운드
				if (RunePlaceFailSound)
				{
					UGameplayStatics::PlaySound2D(this, RunePlaceFailSound);
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

void UGS_ArcaneBoardWidget::RequestShowTooltip(uint8 RuneID, const FVector2D& MousePos)
{
	if (!ShouldShowTooltip())
	{
		return;
	}

	if (IsValid(RuneTooltipWidget))
	{
		if(CurrTooltipRuneID == RuneID)
		{
			FVector2D TooltipPos = CalculateTooltipPosition(MousePos);
			RuneTooltipWidget->SetPositionInViewport(TooltipPos, false);
			return;
		}
		else
		{
			HideTooltip();
		}
	}

	CancelTooltipRequest();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TooltipDelayTimer,
			[this, RuneID, MousePos]()
			{
				ShowTooltip(RuneID, MousePos);
			},
			0.5f,
			false
		);
	}
}

void UGS_ArcaneBoardWidget::HideTooltip()
{
	CancelTooltipRequest();
	CurrTooltipRuneID = 0;

	if (IsValid(RuneTooltipWidget))
	{
		RuneTooltipWidget->RemoveFromParent();
		RuneTooltipWidget = nullptr;
	}
}

void UGS_ArcaneBoardWidget::CancelTooltipRequest()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TooltipDelayTimer);
	}
}

void UGS_ArcaneBoardWidget::BindToLPS()
{
	ArcaneBoardLPS = GetOwningLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>();
	if (IsValid(ArcaneBoardLPS))
	{
		ArcaneBoardLPS->SetCurrUIWidget(this);

		BoardManager = ArcaneBoardLPS->GetOrCreateBoardManager();
		if (IsValid(BoardManager))
		{
			ArcaneBoardLPS->LoadBoardConfig();
			RefreshForCurrCharacter();
		}
	}
}

void UGS_ArcaneBoardWidget::UnbindFromLPS()
{
	if (IsValid(ArcaneBoardLPS))
	{
		ArcaneBoardLPS->ClearCurrUIWidget();
	}
}

FVector2D UGS_ArcaneBoardWidget::GetRuneDragOffset()
{
	FVector2D AdditionalOffset = { 0.f, 0.f };
	FVector2D CellSize = GetGridCellSize();

	if (SelectedRuneID == 1 || SelectedRuneID == 2 || SelectedRuneID == 3
		|| SelectedRuneID == 4 || SelectedRuneID == 5 || SelectedRuneID == 14)
	{
		AdditionalOffset.X = CellSize.X * 0.2f;
		AdditionalOffset.Y = CellSize.X * 0.2f;
	}
	else if (SelectedRuneID == 7)
	{
		AdditionalOffset.X = CellSize.X * 0.3f;
		AdditionalOffset.Y = CellSize.X * 0.3f;
	}
	else if (SelectedRuneID == 8)
	{
		AdditionalOffset.X = CellSize.X * 0.2f;
	}
	else if (SelectedRuneID == 6 || SelectedRuneID == 9)
	{
		AdditionalOffset.X = CellSize.X * 0.2f;
		AdditionalOffset.Y = CellSize.X * 0.3f;
	}
	else if (SelectedRuneID == 11)
	{
		AdditionalOffset.X = CellSize.X * 0.2f;
		AdditionalOffset.Y = CellSize.X * 0.5f;
	}
	else if (SelectedRuneID == 10 || SelectedRuneID == 12)
	{
		AdditionalOffset.X = CellSize.X * 0.3f;
	}
	else if (SelectedRuneID == 13)
	{
		AdditionalOffset.X = CellSize.X * 0.4f;
		AdditionalOffset.Y = CellSize.X * 0.4f;
	}
	else if (SelectedRuneID == 16)
	{
		AdditionalOffset.Y = CellSize.X * 0.3f;
	}

	return AdditionalOffset;
}

void UGS_ArcaneBoardWidget::ShowTooltip(uint8 RuneID, const FVector2D& MousePos)
{
	if (!ShouldShowTooltip() || !IsValid(BoardManager) || !IsValid(TooltipWidgetClass))
	{
		return;
	}

	if (MousePos.X <= 0 && MousePos.Y <= 0)
	{
		return;
	}

	FRuneTableRow RuneData;
	if (!BoardManager->GetRuneData(RuneID, RuneData))
	{
		return;
	}

	HideTooltip();

	RuneTooltipWidget = CreateWidget<UGS_RuneTooltipWidget>(this, TooltipWidgetClass);
	if (RuneTooltipWidget)
	{
		CurrTooltipRuneID = RuneID;
		RuneTooltipWidget->SetRuneData(RuneData);
		RuneTooltipWidget->SetVisibility(ESlateVisibility::Hidden);
		RuneTooltipWidget->AddToViewport(5);

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick([this, MousePos]()
				{
					if (IsValid(RuneTooltipWidget))
					{
						TooltipSize = RuneTooltipWidget->GetDesiredSize();
						FVector2D TooltipPos = CalculateTooltipPosition(MousePos);
						RuneTooltipWidget->SetPositionInViewport(TooltipPos, false);
						RuneTooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
					}
				});
		}
	}
}

FVector2D UGS_ArcaneBoardWidget::CalculateTooltipPosition(const FVector2D& MousePos)
{
	FVector2D TooltipPos = FVector2D(MousePos.X, MousePos.Y - TooltipSize.Y);

	if (APlayerController* PC = GetOwningPlayer())
	{
		int32 ViewportWidth, ViewportHeight;
		PC->GetViewportSize(ViewportWidth, ViewportHeight);
		FVector2D ViewportSize = FVector2D(ViewportWidth, ViewportHeight);

		if (TooltipPos.X + TooltipSize.X > ViewportSize.X)
		{
			TooltipPos.X = MousePos.X - TooltipSize.X;
		}

		if (TooltipPos.Y < 0)
		{
			TooltipPos.Y = MousePos.Y + 20;
		}

		TooltipPos.X = FMath::Clamp(TooltipPos.X, 0.0f, ViewportSize.X - TooltipSize.X);
		TooltipPos.Y = FMath::Clamp(TooltipPos.Y, 0.0f, ViewportSize.Y - TooltipSize.Y);
	}

	return TooltipPos;
}

bool UGS_ArcaneBoardWidget::ShouldShowTooltip() const
{
	if (bIsInSelectionMode && SelectedRuneID > 0)
	{
		return false;
	}

	return true;
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