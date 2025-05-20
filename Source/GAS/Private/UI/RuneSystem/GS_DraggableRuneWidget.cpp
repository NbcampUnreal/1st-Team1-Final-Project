// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_DraggableRuneWidget.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "UI/RuneSystem/GS_RuneDragOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"

UGS_DraggableRuneWidget::UGS_DraggableRuneWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RuneID = 0;
	bIsPlaced = false;
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

FReply UGS_DraggableRuneWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);

	if (bIsPlaced)
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

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ParentBoardWidget)
		{
			ParentBoardWidget->StartRuneSelection(RuneID);
			return FReply::Handled();
		}
	}

	return Reply;
}

void UGS_DraggableRuneWidget::InitRuneWidget(uint8 InRuneID, UTexture2D* InRuneTexture, UGS_ArcaneBoardWidget* BoardWidget)
{
	SetRuneID(InRuneID);
	SetRuneTexture(InRuneTexture);
	ParentBoardWidget = BoardWidget;
}

void UGS_DraggableRuneWidget::SetRuneID(uint8 InRuneID)
{
	RuneID = InRuneID;
}

uint8 UGS_DraggableRuneWidget::GetRuneID() const
{
	return RuneID;
}

void UGS_DraggableRuneWidget::SetRuneTexture(UTexture2D* Texture)
{
	if (IsValid(DragHandleButton) && Texture)
	{
		FButtonStyle ButtonStyle = DragHandleButton->WidgetStyle;
		ButtonStyle.Normal.SetResourceObject(Texture);
		DragHandleButton->SetStyle(ButtonStyle);

		RuneTexture = Texture;
	}
}

void UGS_DraggableRuneWidget::SetPlaced(bool bPlaced)
{
	if (bIsPlaced != bPlaced)
	{
		bIsPlaced = bPlaced;

		if (IsValid(DragHandleButton))
		{
			DragHandleButton->SetIsEnabled(!bIsPlaced);
		}
	}
}
