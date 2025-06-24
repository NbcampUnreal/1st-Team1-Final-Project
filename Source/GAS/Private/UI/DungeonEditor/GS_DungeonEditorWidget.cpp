#include "UI/DungeonEditor/GS_DungeonEditorWidget.h"

#include "CommonButtonBase.h"
#include "DungeonEditor/GS_DEController.h"

void UGS_DungeonEditorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SaveButton)
	{
		if (UCommonButtonBase* SaveButtonBase = Cast<UCommonButtonBase>(SaveButton))
		{
			SaveButtonBase->OnClicked().AddUObject(this, &UGS_DungeonEditorWidget::OnSaveButtonClicked);
		}
	}

	if (LoadButton)
	{
		if (UCommonButtonBase* LoadButtonBase = Cast<UCommonButtonBase>(LoadButton))
		{
			LoadButtonBase->OnClicked().AddUObject(this, &UGS_DungeonEditorWidget::OnLoadButtonClicked);
		}
	}
}

void UGS_DungeonEditorWidget::OnSaveButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			DEPC->GetBuildManager()->SaveDungeonData();
		}
	}
}

void UGS_DungeonEditorWidget::OnLoadButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			DEPC->GetBuildManager()->LoadDungeonData();
		}
	}
}
