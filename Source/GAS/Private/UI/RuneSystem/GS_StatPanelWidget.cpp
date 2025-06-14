// Copyright Epic Games, Inc. All Rights Reserved.


#include "UI/RuneSystem/GS_StatPanelWidget.h"
#include "UI/RuneSystem/GS_StatDataWidget.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_EnumUtils.h"
#include "Components/VerticalBox.h"

UGS_StatPanelWidget::UGS_StatPanelWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UDataTable> StatDataTableAsset(TEXT("/Game/DataTable/StatDataTable.StatDataTable"));
	if (StatDataTableAsset.Succeeded())
	{
		StatDataTable = StatDataTableAsset.Object;
	}

	if (!IsValid(StatWidgetClass))
	{
		FSoftClassPath StatWidgetClassPath(TEXT("/Game/UI/RuneSystem/WBP_StatData.WBP_StatData_C"));
		UClass* LoadedClass = StatWidgetClassPath.TryLoadClass<UGS_StatDataWidget>();

		if (LoadedClass)
		{
			StatWidgetClass = LoadedClass;
		}
	}
}

void UGS_StatPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_StatPanelWidget::InitStatList(UGS_ArcaneBoardManager* InBoardManager)
{
	BoardManager = InBoardManager;

	if (!IsValid(BoardManager) || !IsValid(StatDataTable) || !IsValid(StatDatas) || !IsValid(StatWidgetClass))
	{
		return;
	}

	FString CurrClass = UGS_EnumUtils::GetEnumAsString(BoardManager->CurrClass);
	const FGS_StatRow* FoundRow = StatDataTable->FindRow<FGS_StatRow>(*CurrClass, TEXT("InitStatList"));

	if (!FoundRow)
	{
		return;
	}

	StatDatas->ClearChildren();
	StatDataList.Empty();

	UStruct* StatStruct = FGS_StatRow::StaticStruct();
	for (TFieldIterator<FProperty> PropIt(StatStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
		{
			FName StatName = Property->GetFName();
			float StatValue = *FloatProp->ContainerPtrToValuePtr<float>(FoundRow);
			
			UGS_StatDataWidget* StatWidget = CreateWidget<UGS_StatDataWidget>(this, StatWidgetClass);
			if (StatWidget)
			{
				StatWidget->InitStat(StatName, StatValue);

				StatDatas->AddChild(StatWidget);

				StatDataList.Add(StatName, StatWidget);
			}
		}
	}
}

void UGS_StatPanelWidget::UpdateStats(const FArcaneBoardStats& RuneStats)
{
	if (!IsValid(BoardManager) || !IsValid(StatDataTable))
	{
		return;
	}

	for (auto& StatPair : StatDataList)
	{
		FName StatName = StatPair.Key;
		UGS_StatDataWidget* StatWidget = StatPair.Value;

		if (IsValid(StatWidget))
		{
			float RuneValue = GetStatValueByName(RuneStats.RuneStats, StatName);
			float BonusValue = GetStatValueByName(RuneStats.BonusStats, StatName);

			StatWidget->SetRuneStatData(RuneValue, BonusValue);
		}
	}
}

float UGS_StatPanelWidget::GetStatValueByName(const FGS_StatRow& StatRow, FName StatName)
{
	UStruct* StatStruct = FGS_StatRow::StaticStruct();
	FProperty* Property = StatStruct->FindPropertyByName(StatName);

	if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
	{
		return *FloatProp->ContainerPtrToValuePtr<float>(&StatRow);
	}

	return 0.0f;
}
