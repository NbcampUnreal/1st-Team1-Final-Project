// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Option/GS_OptionMenuUI.h"
#include "UI/Common/CustomCommonButton.h"
#include "UI/Icon/GS_SettingIconButton.h"

void UGS_OptionMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UGS_OptionMenuUI::OnCloseButtonClicked);
	}
}

void UGS_OptionMenuUI::OnCloseButtonClicked()
{
	if (!OwnerUI)
		return;
	
	if (UGS_SettingIconButton* MainMenuUI = Cast<UGS_SettingIconButton>(OwnerUI))
	{
		MainMenuUI->OptionMenuAddressClear();
	}
	RemoveFromParent();
	// ConditionalBeginDestroy();
}

void UGS_OptionMenuUI::SetOwnerUI(UUserWidget* InOwnerUI)
{
	OwnerUI = InOwnerUI;
}