// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_MiniPortrait.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UGS_MiniPortrait::Init(AGS_Monster* Monster)
{
	PortraitImage->SetBrushFromTexture(Monster->GetPortrait());

	if (UGS_StatComp* StatComp = Monster->GetStatComp())
	{
		StatComp->OnCurrentHPChanged.AddUObject(this, &UGS_MiniPortrait::OnHPChanged);
		OnHPChanged(StatComp); 
	}
}

void UGS_MiniPortrait::OnHPChanged(UGS_StatComp* InStatComp)
{
	HPText->SetText(FText::FromString(FString::Printf(TEXT("%d"),FMath::RoundToInt(InStatComp->GetCurrentHealth()))));
}
