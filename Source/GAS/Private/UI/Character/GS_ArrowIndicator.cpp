// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/Character/GS_ArrowIndicator.h"
#include "Components/Image.h"

UGS_ArrowIndicator::UGS_ArrowIndicator(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsActive = true;
	ActiveOpacity = 0.7f;
	InactiveOpacity = 0.2f;
}

void UGS_ArrowIndicator::NativeConstruct()
{
	Super::NativeConstruct();

	if (Indicator)
	{
		Indicator->SetOpacity(ActiveOpacity);
	}
}

void UGS_ArrowIndicator::SetActive(bool bActive)
{
	if (!Indicator)
	{
		return;
	}

	bIsActive = bActive;
	float TargetOpacity = bActive ? ActiveOpacity : InactiveOpacity;
	Indicator->SetOpacity(TargetOpacity);
}

void UGS_ArrowIndicator::SetArrowTexture(UTexture2D* NewTexture)
{
	if (Indicator && NewTexture)
	{
		Indicator->SetBrushFromTexture(NewTexture);
	}
}
