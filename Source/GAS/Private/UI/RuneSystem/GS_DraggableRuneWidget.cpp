// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_DraggableRuneWidget.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "Components/Image.h"

UGS_DraggableRuneWidget::UGS_DraggableRuneWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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

	RuneID = 0;
	bIsPlaced = false;
}

void UGS_DraggableRuneWidget::NativeConstruct()
{
	Super::NativeConstruct();
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
		if (IsValid(SelectionIndicator))
		{
			if (SelectionIndicator->GetVisibility() == ESlateVisibility::Visible)
			{
				return FReply::Handled();;
			}
		}

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
		SetRuneVisualState(true, bIsPlaced);
	}
}

void UGS_DraggableRuneWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (!bIsPlaced)
	{
		SetRuneVisualState(false, bIsPlaced);
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
	if (!IsValid(RuneState))
	{
		return;
	}

	FLinearColor NewColor;
	if (bDisabled)
	{
		NewColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.7f);
		if (IsValid(SelectionIndicator))
		{
			SelectionIndicator->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (bHovered)
	{
		NewColor = FLinearColor(1.2f, 1.2f, 1.0f, 0.7f);
	}
	else
	{
		NewColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		if (IsValid(SelectionIndicator))
		{
			SelectionIndicator->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	RuneState->SetColorAndOpacity(NewColor);
}

void UGS_DraggableRuneWidget::SetPlaced(bool bPlaced)
{
	if (bIsPlaced != bPlaced)
	{
		bIsPlaced = bPlaced;
		SetRuneVisualState(false, bIsPlaced);
	}
}