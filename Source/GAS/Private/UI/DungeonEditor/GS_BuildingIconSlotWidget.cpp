#include "UI/DungeonEditor/GS_BuildingIconSlotWidget.h"

#include "Components/Button.h"
#include "DungeonEditor/GS_DEController.h"

void UGS_BuildingIconSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TempBtn)
	{
		TempBtn->OnClicked.AddDynamic(this, &UGS_BuildingIconSlotWidget::PressedTempBtn);
	}
}

void UGS_BuildingIconSlotWidget::PressedTempBtn()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			DEPC->GetBuildManager()->EnableBuildingTool(&Data);
		}
	}
}