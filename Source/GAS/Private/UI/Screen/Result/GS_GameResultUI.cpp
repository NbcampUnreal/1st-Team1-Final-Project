// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Result/GS_GameResultUI.h"
#include "UI/Screen/Result/GS_GameResultBoardUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "CommonUI/Public/CommonButtonBase.h"
#include "UI/Common/CustomCommonButton.h"
#include "System/GS_PlayerState.h"
#include "System/PlayerController/GS_ResultPC.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

void UGS_GameResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked().AddUObject(this, &UGS_GameResultUI::OnContinueButtonClicked);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGS_GameResultUI::AddResultBoards, 1.f, false);
}

void UGS_GameResultUI::OnContinueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("continuebuttonclicked"));

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AGS_ResultPC* GS_PC = Cast<AGS_ResultPC>(PC);
		if (GS_PC)
		{
			GS_PC->Server_RequestTravelToLobby();
		}
	}
}


void UGS_GameResultUI::AddResultBoards()
{
	UE_LOG(LogTemp, Warning, TEXT("UGS_GameResultUI::AddResultBoards - Executed!!!!!!!!!!"));
	if (!GameResultBoardUIClass || !PawnMappingDataAsset || !PlayerResultHorizontalBox) return;

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS || GS->PlayerArray.Num() == 0) return;

	PlayerResultHorizontalBox->ClearChildren();

	AGS_PlayerState* GuardianState = nullptr;
	TArray<AGS_PlayerState*> SeekerStates;

	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		if (AGS_PlayerState* GSPlayerState = Cast<AGS_PlayerState>(PlayerState))
		{
			if (GSPlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
			{
				GuardianState = GSPlayerState;
			}
			else if (GSPlayerState->CurrentPlayerRole == EPlayerRole::PR_Seeker)
			{
				SeekerStates.Add(GSPlayerState);
			}
		}
	}

	if (GuardianState)
	{
		UGS_GameResultBoardUI* GuardianBoard = CreateWidget<UGS_GameResultBoardUI>(this, GameResultBoardUIClass);
		if (GuardianBoard)
		{
			GuardianBoard->SetupResultBoard(GuardianState, PawnMappingDataAsset);
			UHorizontalBoxSlot* HBoxSlot = PlayerResultHorizontalBox->AddChildToHorizontalBox(GuardianBoard);
			if (HBoxSlot)
			{
				HBoxSlot->SetPadding(FMargin(0.f, 0.f, 30.f, 0.f));
			}
		}
	}

	for (AGS_PlayerState* SeekerState : SeekerStates)
	{
		UGS_GameResultBoardUI* SeekerBoard = CreateWidget<UGS_GameResultBoardUI>(this, GameResultBoardUIClass);
		if (SeekerBoard)
		{
			SeekerBoard->SetupResultBoard(SeekerState, PawnMappingDataAsset);
			UHorizontalBoxSlot* HBoxSlot = PlayerResultHorizontalBox->AddChildToHorizontalBox(SeekerBoard);
			if (HBoxSlot)
			{
				HBoxSlot->SetPadding(FMargin(0.f, 0.f, 30.f, 0.f));
			}
		}
	}


	//텍스트
	AGS_PlayerState* APS = GetOwningPlayer()->GetPlayerState<AGS_PlayerState>();
	EPlayerRole MyRole = APS->CurrentPlayerRole;
	EGameResult Result = APS->CurrentGameResult;

	if (MyRole == EPlayerRole::PR_Guardian)
	{
		Text_MyRole->SetText(FText::FromString(TEXT("Guardian")));

		if (Result == EGameResult::GR_SeekersLost)
		{
			Text_Victory->SetText(FText::FromString(TEXT("Victory")));
		}
		else if (Result == EGameResult::GR_SeekersWon)
		{
			Text_Victory->SetText(FText::FromString(TEXT("Defeat")));
		}
		else
		{
			Text_Victory->SetText(FText::FromString(TEXT("ERROR")));
		}
	}
	else
	{
		Text_MyRole->SetText(FText::FromString(TEXT("Seeker")));

		if (Result == EGameResult::GR_SeekersLost)
		{
			Text_Victory->SetText(FText::FromString(TEXT("Defeat")));
		}
		else if (Result == EGameResult::GR_SeekersWon)
		{
			Text_Victory->SetText(FText::FromString(TEXT("Victory")));
		}
		else
		{
			Text_Victory->SetText(FText::FromString(TEXT("ERROR")));
		}
	}


}
