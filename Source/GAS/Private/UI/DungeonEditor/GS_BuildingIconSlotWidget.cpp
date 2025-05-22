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

void UGS_BuildingIconSlotWidget::SetButtonImage()
{
	const FGS_PlaceableObjectsRow* RowPtr = Data.GetRow<FGS_PlaceableObjectsRow>(Data.RowName.ToString());
	if (nullptr != RowPtr)
	{
		TObjectPtr<UTexture> Icon = *RowPtr->Icon;
		FButtonStyle ButtonStyle;
		ButtonStyle.Normal.SetResourceObject(Icon);
		ButtonStyle.Normal.ImageSize = FVector2D(100, 100);
		ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.495466,0.495466,0.495466, 1.000000));
		ButtonStyle.Hovered.SetResourceObject(Icon);
		ButtonStyle.Hovered.ImageSize = FVector2D(100, 100);
		ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.724268,0.724268,0.724268, 1.000000));
		ButtonStyle.Pressed.SetResourceObject(Icon);
		ButtonStyle.Pressed.ImageSize = FVector2D(100, 100);
		ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.384266,0.384266,0.384266, 1.000000));
		
		TempBtn->SetStyle(ButtonStyle);
	}
}
