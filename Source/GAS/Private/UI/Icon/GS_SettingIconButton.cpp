#include "UI/Icon/GS_SettingIconButton.h"

#include "CommonButtonBase.h"
#include "UI/Screen/GS_MainMenuUI.h"
#include "UI/Screen/Option/GS_OptionMenuUI.h"

void UGS_SettingIconButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (SettingButton)
	{
		if (UCommonButtonBase* SettingButtonBase = Cast<UCommonButtonBase>(SettingButton))
		{
			SettingButtonBase->OnClicked().AddUObject(this, &UGS_SettingIconButton::OnSettingButtonClicked);
		}
	}
}

void UGS_SettingIconButton::OnSettingButtonClicked()
{
	if (!OptionMenuUI)
	{
		OptionMenuUI = CreateWidget<UGS_OptionMenuUI>(this, OptionMenuUIClass);
		OptionMenuUI->AddToViewport();
		OptionMenuUI->SetOwnerUI(this);
	}
}

void UGS_SettingIconButton::OptionMenuAddressClear()
{
	if (OptionMenuUI)
	{
		OptionMenuUI = nullptr;
	}
}
