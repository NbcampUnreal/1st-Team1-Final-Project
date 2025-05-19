#include "UI/DungeonEditor/GS_PropsSelector.h"

#include "Components/UniformGridPanel.h"
#include "DungeonEditor/GS_DEController.h"
#include "UI/DungeonEditor/GS_BuildingIconSlotWidget.h"

void UGS_PropsSelector::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			if (AGS_BuildManager* BuildManagerRef = DEPC->GetBuildManager())
			{
				if (TArray<FDataTableRowHandle>* DataTableRows = BuildManagerRef->GetBuildingsRows())
				{
					IconsAmount = DataTableRows->Num();
					for (int i = 0; i < IconsAmount; ++i)
					{
						FDataTableRowHandle Data = DataTableRows->GetData()[i];
						UGS_BuildingIconSlotWidget* NewWidget = CreateWidget<UGS_BuildingIconSlotWidget>(this, IconSlotWidgetClass);
						if (nullptr != NewWidget)
						{
							NewWidget->Data = Data;
							NewWidget->SetButtonImage();

							int IdxX = i / SlotCount;
							int IdxY = i % SlotCount;
							GridSlot->AddChildToUniformGrid(NewWidget, IdxX, IdxY);
						}
					}
				}
				
			}
		}
	}
}
