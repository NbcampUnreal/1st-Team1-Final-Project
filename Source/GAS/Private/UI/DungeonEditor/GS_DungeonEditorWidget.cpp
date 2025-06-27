#include "UI/DungeonEditor/GS_DungeonEditorWidget.h"

#include "CommonButtonBase.h"
#include "DungeonEditor/GS_DEController.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"

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

	if (BackButton)
	{
		if (UCommonButtonBase* BackButtonBase = Cast<UCommonButtonBase>(BackButton))
		{
			BackButtonBase->OnClicked().AddUObject(this, &UGS_DungeonEditorWidget::OnBackButtonClicked);
		}
	}
	
	if (ResetButton)
	{
		if (UCommonButtonBase* ResetButtonBase = Cast<UCommonButtonBase>(ResetButton))
		{
			ResetButtonBase->OnClicked().AddUObject(this, &UGS_DungeonEditorWidget::OnResetButtonClicked);
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

void UGS_DungeonEditorWidget::OnBackButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AGS_CustomLobbyPC* CustomLobbyPC = Cast<AGS_CustomLobbyPC>(PC))
		{
			CustomLobbyPC->RequestDungeonEditorToLobby();
		}
	}
}

void UGS_DungeonEditorWidget::OnResetButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			DEPC->GetBuildManager()->ResetDungeonData();
		}
	}
}
