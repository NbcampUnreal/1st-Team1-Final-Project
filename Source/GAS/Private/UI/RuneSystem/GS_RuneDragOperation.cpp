// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_RuneDragOperation.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"

void UGS_RuneDragOperation::InitDragOp(uint8 InRuneID, UUserWidget* InDragVisual)
{
	RuneID = InRuneID;
	DefaultDragVisual = InDragVisual;
	Pivot = EDragPivot::CenterCenter;
}
