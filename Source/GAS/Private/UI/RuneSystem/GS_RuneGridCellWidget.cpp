// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_RuneGridCellWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"

UGS_RuneGridCellWidget::UGS_RuneGridCellWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	VisualState = EGridCellVisualState::Normal;
}

void UGS_RuneGridCellWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ParentBoardWidget = Cast<UGS_ArcaneBoardWidget>(GetParent()->GetParent());
}

void UGS_RuneGridCellWidget::SetCellData(const FGridCellData& InCellData)
{
	CellData = InCellData;

	switch (CellData.State)
	{
	case EGridCellState::Occupied:
		SetVisualState(EGridCellVisualState::Occupied);
		break;
	case EGridCellState::Locked:
		SetVisualState(EGridCellVisualState::Invalid);
		break;
	default:
		SetVisualState(EGridCellVisualState::Normal);
		break;
	}

	if (CellData.bIsSpecialCell)
	{
		OnVisualStateChanged(EGridCellVisualState::Special);
	}
}

FIntPoint UGS_RuneGridCellWidget::GetCellPos() const
{
	return CellData.Pos;
}

void UGS_RuneGridCellWidget::SetVisualState(EGridCellVisualState NewState)
{
	if (VisualState != NewState)
	{
		VisualState = NewState;

		OnVisualStateChanged(NewState);
	}
}

void UGS_RuneGridCellWidget::SetRuneTexture(UTexture2D* Texture)
{
	if (IsValid(DropZone))
	{
		FButtonStyle ButtonStyle = DropZone->WidgetStyle;
		ButtonStyle.Normal.SetResourceObject(Texture);
		DropZone->SetStyle(ButtonStyle);
	}
}
