#include "UI/DungeonEditor/GS_PropsSelector.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "UI/DungeonEditor/GS_BuildingIconSlotWidget.h"

void UGS_PropsSelector::NativeConstruct()
{
	Super::NativeConstruct();

}


void UGS_PropsSelector::SetTitle(FText InText)
{
	GridName->SetText(InText);
}

void UGS_PropsSelector::CreateIconSlot(const FDataTableRowHandle& InData)
{
	UGS_BuildingIconSlotWidget* NewWidget = CreateWidget<UGS_BuildingIconSlotWidget>(this, IconSlotWidgetClass);
	if (nullptr != NewWidget)
	{
		NewWidget->Data = InData;
		NewWidget->SetButtonImage();

		int IdxX = IconsAmount / SlotCount;
		int IdxY = IconsAmount % SlotCount;
		GridSlot->AddChildToUniformGrid(NewWidget, IdxX, IdxY);
		IconsAmount++;
	}
}
