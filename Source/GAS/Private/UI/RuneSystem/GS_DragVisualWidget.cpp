// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_DragVisualWidget.h"
#include "Components/Image.h"

UGS_DragVisualWidget::UGS_DragVisualWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RuneID = 0;
}

void UGS_DragVisualWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_DragVisualWidget::Setup(uint8 InRuneID, UTexture2D* InTexture)
{
	RuneID = InRuneID;

	if (IsValid(RuneImage) && InTexture)
	{
		RuneImage->SetBrushFromTexture(InTexture);
	}
}
