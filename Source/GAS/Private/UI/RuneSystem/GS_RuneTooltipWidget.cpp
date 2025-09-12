// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_RuneTooltipWidget.h"
#include "Components/TextBlock.h"

UGS_RuneTooltipWidget::UGS_RuneTooltipWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGS_RuneTooltipWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_RuneTooltipWidget::SetRuneData(const FRuneTableRow& RuneData)
{
	CurrRuneData = RuneData;
	UpdateTooltipUI();
}

uint8 UGS_RuneTooltipWidget::GetRuneID()
{
	return CurrRuneData.RuneID;
}

void UGS_RuneTooltipWidget::UpdateTooltipUI()
{
	if (IsValid(RuneName))
	{
		//룬 네임 추가 후 변경
		//RuneName->SetText(RuneData.RuneName);
		RuneName->SetText(FText::AsNumber(CurrRuneData.RuneID));
	}

	if (IsValid(RuneDescription))
	{
		RuneDescription->SetText(CurrRuneData.Description);
	}

	if (IsValid(RuneStat))
	{
		FString StatString = FString::Printf(TEXT("+%.0f %s"),
			CurrRuneData.StatEffect.Value,
			*CurrRuneData.StatEffect.StatName.ToString());

		RuneStat->SetText(FText::FromString(StatString));
		RuneStat->SetColorAndOpacity(FLinearColor::Green);
	}
}
