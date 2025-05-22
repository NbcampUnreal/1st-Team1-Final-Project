// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_DraggableRuneWidget.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "UI/RuneSystem/GS_RuneDragOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"

UGS_DraggableRuneWidget::UGS_DraggableRuneWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RuneID = 0;
	bIsPlaced = false;
	if (IsValid(SelectionIndicator))
	{

	}
}

void UGS_DraggableRuneWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
}

FReply UGS_DraggableRuneWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (bIsPlaced || !ParentBoardWidget)
	{
		return Reply;
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (ParentBoardWidget)
		{
			ParentBoardWidget->StartRuneSelection(RuneID);
			return FReply::Handled();
		}
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ParentBoardWidget)
		{
			ParentBoardWidget->EndRuneSelection(false);
			return FReply::Handled();
		}
	}

	return Reply;
}

void UGS_DraggableRuneWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!bIsPlaced)
	{
		SetRuneVisualState(true, false);
	}
}

void UGS_DraggableRuneWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (!bIsPlaced)
	{
		SetRuneVisualState(false, false);
	}
}

void UGS_DraggableRuneWidget::InitRuneWidget(uint8 InRuneID, UTexture2D* InRuneTexture, UGS_ArcaneBoardWidget* BoardWidget)
{
	RuneID = InRuneID;
	SetRuneTexture(InRuneTexture);
	ParentBoardWidget = BoardWidget;
}

uint8 UGS_DraggableRuneWidget::GetRuneID() const
{
	return RuneID;
}

void UGS_DraggableRuneWidget::SetRuneTexture(UTexture2D* Texture)
{
	if (IsValid(RuneImage) && Texture)
	{
		RuneImage->SetBrushFromTexture(Texture);
	}
}

void UGS_DraggableRuneWidget::SetRuneVisualState(bool bHovered, bool bDisabled)
{
	if (!IsValid(RuneImage))
	{
		return;
	}

	FLinearColor NewColor;
	if (bDisabled)
	{
		NewColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.7f);
	}
	else if (bHovered)
	{
		NewColor = FLinearColor(1.2f, 1.2f, 1.0f, 1.0f);
	}
	else
	{
		NewColor = FLinearColor::White;
	}

	RuneImage->SetBrushTintColor(NewColor);
}

void UGS_DraggableRuneWidget::SetPlaced(bool bPlaced)
{
	if (bIsPlaced != bPlaced)
	{
		bIsPlaced = bPlaced;
		SetRuneVisualState(false, bIsPlaced);
	}
}