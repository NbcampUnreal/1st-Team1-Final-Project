// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_StatDataWidget.h"
#include "Components/TextBlock.h"

UGS_StatDataWidget::UGS_StatDataWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	BaseValue = 0.0f;
	CurrRuneValue = 0.0f;
	CurrBonusValue = 0.0f;
}

void UGS_StatDataWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_StatDataWidget::InitStat(FName InStatName, float InBaseValue)
{
	StatName = InStatName;
	BaseValue = InBaseValue;
	
	if (IsValid(StatLabel))
	{
		StatLabel->SetText(FText::FromName(StatName));
	}

	if (IsValid(DefaultValue))
	{
		DefaultValue->SetText(FText::AsNumber(BaseValue));
	}

	UpdateUI();
}

void UGS_StatDataWidget::SetAllStatData(FName InStatName, float InDefaultValue, float InRuneValue, float InBonusValue)
{
	StatName = InStatName;
	BaseValue = InDefaultValue;
	CurrRuneValue = InRuneValue;
	CurrBonusValue = InBonusValue;

	if (IsValid(StatLabel))
	{
		StatLabel->SetText(FText::FromName(StatName));
	}

	if (IsValid(DefaultValue))
	{
		DefaultValue->SetText(FText::AsNumber(BaseValue));
	}

	UpdateUI();
}

void UGS_StatDataWidget::SetRuneStatData(float InRuneValue, float InBonusValue)
{
	if (CurrRuneValue != InRuneValue || CurrBonusValue != InBonusValue)
	{
		CurrRuneValue = InRuneValue;
		CurrBonusValue = InBonusValue;
		
		UpdateUI();
	}
}

void UGS_StatDataWidget::UpdateUI()
{
	if (IsValid(RuneValue))
	{
		if (CurrRuneValue > 0)
		{
			RuneValue->SetText(FText::FromString(FString::Printf(TEXT("+%.0f"), CurrRuneValue)));
			RuneValue->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			RuneValue->SetText(FText::FromString(TEXT("+0")));
			RuneValue->SetColorAndOpacity(FLinearColor::Gray);
		}
	}

	if (IsValid(BonusValue))
	{
		if (CurrBonusValue > 0)
		{
			BonusValue->SetText(FText::FromString(FString::Printf(TEXT("+%.0f"), CurrBonusValue)));
			BonusValue->SetColorAndOpacity(FLinearColor::Blue);
		}
		else
		{
			BonusValue->SetText(FText::FromString(TEXT("+0")));
			BonusValue->SetColorAndOpacity(FLinearColor::Gray);
		}
	}

	if (IsValid(TotalValue))
	{
		float TotalVal = BaseValue + CurrRuneValue + CurrBonusValue;
		TotalValue->SetText(FText::AsNumber(TotalVal));

		if (TotalVal > BaseValue)
		{
			TotalValue->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			TotalValue->SetColorAndOpacity(FLinearColor::Black);
		}
	}
}
