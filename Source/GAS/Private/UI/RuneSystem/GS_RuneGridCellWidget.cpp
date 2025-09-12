// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_RuneGridCellWidget.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UGS_RuneGridCellWidget::UGS_RuneGridCellWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	VisualState = EGridCellVisualState::Normal;
}

void UGS_RuneGridCellWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_RuneGridCellWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (ParentBoardWidget)
	{
		if(CellData.PlacedRuneID > 0)
		{
			FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();
			if (APlayerController* PC = GetOwningPlayer())
			{
				FGeometry ScreenGeometry = UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(PC);
				MousePos = ScreenGeometry.AbsoluteToLocal(MousePos);
			}
			ParentBoardWidget->RequestShowTooltip(CellData.PlacedRuneID, MousePos);
		}
		else
		{
			ParentBoardWidget->HideTooltip();
		}
	}
}

void UGS_RuneGridCellWidget::InitCell(const FGridCellData& InCellData, UGS_ArcaneBoardWidget* InParentBoard)
{
	ParentBoardWidget = InParentBoard;
	SetCellData(InCellData);
	PreviewImage->SetVisibility(ESlateVisibility::Hidden);
}

void UGS_RuneGridCellWidget::SetCellData(const FGridCellData& InCellData)
{
	CellData = InCellData;

	UTexture2D* TextureToUse = CellData.bIsConnected && CellData.ConnectedRuneTextureFrag ?
		CellData.ConnectedRuneTextureFrag : CellData.RuneTextureFrag;

	SetRuneTexture(TextureToUse);

	if (CellData.bIsSpecialCell)
	{
		CellBG->SetColorAndOpacity(FLinearColor(0.f, 0.f, 1.f, 0.5f));
	}
}

FIntPoint UGS_RuneGridCellWidget::GetCellPos() const
{
	return CellData.Pos;
}

uint8 UGS_RuneGridCellWidget::GetPlacedRuneID() const
{
	return CellData.PlacedRuneID;
}

void UGS_RuneGridCellWidget::SetPreviewVisualState(EGridCellVisualState NewState)
{
	if (VisualState != NewState)
	{
		VisualState = NewState;

		switch (NewState)
		{
		case EGridCellVisualState::Normal:
			CellBG->SetVisibility(ESlateVisibility::Visible);
			PreviewImage->SetVisibility(ESlateVisibility::Hidden);
			break;
		case EGridCellVisualState::Valid:
			CellBG->SetVisibility(ESlateVisibility::Hidden);
			PreviewImage->SetVisibility(ESlateVisibility::Visible);
			PreviewImage->SetColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 0.2f));
			break;
		case EGridCellVisualState::ReplaceExisting:
			CellBG->SetVisibility(ESlateVisibility::Hidden);
			PreviewImage->SetVisibility(ESlateVisibility::Visible);
			PreviewImage->SetColorAndOpacity(FLinearColor(1.f, 0.65f, 0.f, 0.2f)); // 주황색
			break;
		case EGridCellVisualState::Invalid:
			CellBG->SetVisibility(ESlateVisibility::Hidden);
			PreviewImage->SetVisibility(ESlateVisibility::Visible);
			PreviewImage->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 0.2f));
			break;
		default:
			break;
		}
	}
}

void UGS_RuneGridCellWidget::SetRuneTexture(UTexture2D* Texture)
{
	if (IsValid(RuneImage))
	{
		if (Texture)
		{
			RuneImage->SetBrushFromTexture(Texture);
			RuneImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			RuneImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
