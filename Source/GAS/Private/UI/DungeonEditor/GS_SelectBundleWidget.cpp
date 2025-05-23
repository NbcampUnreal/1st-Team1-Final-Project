#include "UI/DungeonEditor/GS_SelectBundleWidget.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "DungeonEditor/GS_DEController.h"
#include "UI/DungeonEditor/GS_PropsSelector.h"
#include "RuneSystem/GS_EnumUtils.h"

void UGS_SelectBundleWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGS_SelectBundleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	//PropsSelectorWidgets.SetNum(UGS_EnumUtils::GetEnumCount<ETrapPlacement>() - 1);
}

void UGS_SelectBundleWidget::SetTitle(FText InTitle)
{
	if (BandleTilte)
	{
		BandleTilte->SetText(InTitle);
	}
}

void UGS_SelectBundleWidget::CreateSelector(FDataTableRowHandle& InData)
{
	FGS_PlaceableObjectsRow* RowPtr = InData.GetRow<FGS_PlaceableObjectsRow>(InData.RowName.ToString());
	int WidgetIdx = 0;
	ERoomType RoomType = ERoomType::None;
	ETrapPlacement TrapType = ETrapPlacement::Floor;
	EMonsterType MonsterType = EMonsterType::None;
	FText Title;
	
	switch (RowPtr->ObjectType)
	{
	case EObjectType::Room:
		RoomType = RowPtr->RoomType;
		WidgetIdx = (int)RoomType;
		Title = UGS_EnumUtils::GetEnumAsText<ERoomType>(RoomType);
		break;

	case EObjectType::Trap:
		TrapType = RowPtr->TrapType;
		WidgetIdx = (int)TrapType;
		Title = UGS_EnumUtils::GetEnumAsText<ETrapPlacement>(TrapType);
		break;

	case EObjectType::Monster:
		MonsterType = RowPtr->MonsterType;
		WidgetIdx = (int)MonsterType;
		Title = UGS_EnumUtils::GetEnumAsText<EMonsterType>(MonsterType);
		break;

	default:
		break;
	}
	
	if (!PropsSelectorWidgets[WidgetIdx])
	{
		UGS_PropsSelector* NewSelectWidget = CreateWidget<UGS_PropsSelector>(this, PropsSelectorWidgetClass);
		NewSelectWidget->CreateIconSlot(InData);
		NewSelectWidget->SetTitle(Title);
		ScrollBox->AddChild(NewSelectWidget);
		PropsSelectorWidgets[WidgetIdx] = NewSelectWidget;
	}
	else
	{
		if (PropsSelectorWidgets[WidgetIdx])
		{
			PropsSelectorWidgets[WidgetIdx]->CreateIconSlot(InData);
		}
	}
}

void UGS_SelectBundleWidget::InitWidget(EObjectType InObjectType)
{
	switch (InObjectType)
	{
	case EObjectType::Room:
		PropsSelectorWidgets.SetNum(UGS_EnumUtils::GetEnumCount<ERoomType>() - 1);
		break;

	case EObjectType::Trap:
		PropsSelectorWidgets.SetNum(UGS_EnumUtils::GetEnumCount<ETrapPlacement>() - 1);
		break;

	case EObjectType::Monster:
		PropsSelectorWidgets.SetNum(UGS_EnumUtils::GetEnumCount<EMonsterType>() - 1);
		break;

	default:
		break;
	}
}