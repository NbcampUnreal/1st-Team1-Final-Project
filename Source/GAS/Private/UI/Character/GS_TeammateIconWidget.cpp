// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_TeammateIconWidget.h"
#include "UI/Character/GS_CompassIndicatorComponent.h"
#include "Components/Image.h"

void UGS_TeammateIconWidget::SetIconAppearance(UGS_CompassIndicatorComponent* Indicator)
{
	if (!Indicator || !IconImage)
	{
		return;
	}

	if (UTexture2D* IconTexture = Indicator->GetCustomIcon())
	{
		IconImage->SetBrushFromTexture(IconTexture);
	}
	IconImage->SetColorAndOpacity(Indicator->GetIconColor());
} 