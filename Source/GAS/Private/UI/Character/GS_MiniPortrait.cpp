// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_MiniPortrait.h"
#include "Components/Image.h"

void UGS_MiniPortrait::Init(UTexture2D* InPortrait)
{
	if (PortraitImage)
	{
		PortraitImage->SetBrushFromTexture(InPortrait);
	}
}
