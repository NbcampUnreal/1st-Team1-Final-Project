// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Option/GS_OptionMenuUI.h"
#include "UI/Common/CustomCommonButton.h"
#include "UI/Icon/GS_SettingIconButton.h"
#include "UI/Interface/GS_SettingsApplyInterface.h"
#include "Blueprint/WidgetTree.h"

void UGS_OptionMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 설정 적용 플래그 초기화
	bSettingsApplied = false;

	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UGS_OptionMenuUI::OnCloseButtonClicked);
	}
}

void UGS_OptionMenuUI::OnCloseButtonClicked()
{
	// 설정이 적용되지 않은 경우에만 CancelSettings 호출
	if (!bSettingsApplied)
	{
		CancelAllSettings();
	}

	if (!OwnerUI)
	{
		return;
	}

	if (UGS_SettingIconButton* MainMenuUI = Cast<UGS_SettingIconButton>(OwnerUI))
	{
		MainMenuUI->OptionMenuAddressClear();
	}

	this->RemoveFromParent();
}

void UGS_OptionMenuUI::SetOwnerUI(UUserWidget* InOwnerUI)
{
	OwnerUI = InOwnerUI;
}

void UGS_OptionMenuUI::CancelAllSettings()
{
	// WidgetTree에서 모든 위젯 가져오기
	TArray<UWidget*> AllWidgets;
	if (WidgetTree)
	{
		WidgetTree->GetAllWidgets(AllWidgets);
	}

	// IGS_SettingsApplyInterface를 구현한 위젯들의 CancelSettings() 호출
	for (UWidget* Widget : AllWidgets)
	{
		if (Widget && Widget->Implements<UGS_SettingsApplyInterface>())
		{
			IGS_SettingsApplyInterface::Execute_CancelSettings(Widget);
		}
	}
}

void UGS_OptionMenuUI::MarkSettingsApplied()
{
	bSettingsApplied = true;
}