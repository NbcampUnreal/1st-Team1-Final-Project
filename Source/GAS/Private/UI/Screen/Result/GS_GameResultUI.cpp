// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Result/GS_GameResultUI.h"
#include "Components/Button.h"
#include "CommonUI/Public/CommonButtonBase.h"
#include "UI/Common/CustomCommonButton.h"

void UGS_GameResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked().AddUObject(this, &UGS_GameResultUI::OnContinueButtonClicked);
	}
}

void UGS_GameResultUI::OnContinueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("continuebuttonclicked"));
}


void UGS_GameResultUI::AddResultBoards()
{
	if (!ResultBoardOverlay) return;

	ResultBoardOverlay->ClearChildren();

	UClass* BoardClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/UI/LevelUI/Result/WBP_GameResultBoard.WBP_GameResultBoard_C"));
	if(BoardClass)
	{
		UGS_GameResultBoardUI* BoardWidget = CreateWidget<UGS_GameResultBoardUI>(GetWorld(), BoardClass);
		if (BoardWidget)
		{
			ResultBoardOverlay->AddChildToOverlay(BoardWidget);
		}
	}


}
