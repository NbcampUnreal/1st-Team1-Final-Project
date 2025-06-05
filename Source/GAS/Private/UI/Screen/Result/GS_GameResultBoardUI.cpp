// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/Screen/Result/GS_GameResultBoardUI.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/Common/CustomCommonButton.h"


void UGS_GameResultBoardUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ArcaneBoardButton)
	{
		ArcaneBoardButton->OnClicked().AddUObject(this, &UGS_GameResultBoardUI::OnArcaneBoardButtonClicked);
	}
}

void UGS_GameResultBoardUI::OnArcaneBoardButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("OnArcaneBoardButtonClicked"));
}