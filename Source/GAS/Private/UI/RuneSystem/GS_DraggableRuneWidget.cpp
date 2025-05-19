// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_DraggableRuneWidget.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "Components/Button.h"

UGS_DraggableRuneWidget::UGS_DraggableRuneWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RuneID = 0;
}

void UGS_DraggableRuneWidget::NativeConstruct()
{
	Super::NativeConstruct();
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
	if (IsValid(DragHandleButton))
	{
		FButtonStyle ButtonStyle = DragHandleButton->WidgetStyle;
		ButtonStyle.Normal.SetResourceObject(Texture);
		DragHandleButton->SetStyle(ButtonStyle);
	}
}
