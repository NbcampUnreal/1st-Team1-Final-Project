// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Option/GS_InGameManualWidget.h"
#include "CommonButtonBase.h"

void UGS_InGameManualWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UGS_InGameManualWidget::OnCloseButtonClicked);
	}
}

void UGS_InGameManualWidget::OnCloseButtonClicked()
{
	RemoveFromParent();
}
