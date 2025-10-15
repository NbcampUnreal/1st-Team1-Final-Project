// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Option/GS_InGameMenuUI.h"
#include "CommonUI/Public/CommonButtonBase.h"
#include "UI/Screen/Option/GS_OptionMenuUI.h"
#include "UI/Screen/Option/GS_InGameManualWidget.h"

void UGS_InGameMenuUI::NativeConstruct()
{
	if (ResumeButton)
	{
		if (UCommonButtonBase* ResumeButtonBase = Cast<UCommonButtonBase>(ResumeButton))
		{
			ResumeButtonBase->OnClicked().AddUObject(this, &UGS_InGameMenuUI::OnResumeButtonClicked);
		}
	}

	if (ManualButton)
	{
		if (UCommonButtonBase* ManualButtonBase = Cast<UCommonButtonBase>(ManualButton))
		{
			ManualButtonBase->OnClicked().AddUObject(this, &UGS_InGameMenuUI::OnManualButtonClicked);
		}
	}

	if (SettingButton)
	{
		if (UCommonButtonBase* SettingButtonBase = Cast<UCommonButtonBase>(SettingButton))
		{
			SettingButtonBase->OnClicked().AddUObject(this, &UGS_InGameMenuUI::OnSettingButtonClicked);
		}
	}

	if (ExitButton)
	{
		if (UCommonButtonBase* ExitButtonBase = Cast<UCommonButtonBase>(ExitButton))
		{
			ExitButtonBase->OnClicked().AddUObject(this, &UGS_InGameMenuUI::OnExitButtonClicked);
		}
	}
}

void UGS_InGameMenuUI::OnResumeButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGS_InGameMenuUI::OnManualButtonClicked()
{
	if (ManualUIClass)
	{
		UGS_InGameManualWidget* ManualUI = CreateWidget<UGS_InGameManualWidget>(this, ManualUIClass);
		ManualUI->AddToViewport(2);
	}
}

void UGS_InGameMenuUI::OnSettingButtonClicked()
{
	if(OptionMenuUIClass)
	{
		UGS_OptionMenuUI* OptionMenuUI = CreateWidget<UGS_OptionMenuUI>(this, OptionMenuUIClass);
		OptionMenuUI->AddToViewport(2);
		OptionMenuUI->SetOwnerUI(this);
	}
}

void UGS_InGameMenuUI::OnExitButtonClicked()
{
	//exit game
}
