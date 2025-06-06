// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/Result/GS_GameResultUI.h"
#include "UI/Screen/Result/GS_GameResultBoardUI.h"
#include "Components/Button.h"
#include "CommonUI/Public/CommonButtonBase.h"
#include "UI/Common/CustomCommonButton.h"
#include "System/GS_PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UGS_GameResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked().AddUObject(this, &UGS_GameResultUI::OnContinueButtonClicked);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGS_GameResultUI::AddResultBoards, 0.3f, false);
}

void UGS_GameResultUI::OnContinueButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("continuebuttonclicked"));
}


void UGS_GameResultUI::AddResultBoards()
{
	UE_LOG(LogTemp, Warning, TEXT("UGS_GameResultUI::AddResultBoards - Executed!!!!!!!!!!"));
	if (!GameResultBoardUIClass) return;
	if (!PawnMappingDataAsset) return;
	if (!GuardianResultBoardOverlay || !SeekerResultBoardOverlay) return;

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	if (GS->PlayerArray.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PopulateResultBoards: PlayerArray is still empty after delay."));
		return;
	}


	GuardianResultBoardOverlay->ClearChildren();
	SeekerResultBoardOverlay->ClearChildren();

	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		UE_LOG(LogTemp, Warning, TEXT("$$$$$$$$$$$$ PlayerArray.Num() = %d"), GS->PlayerArray.Num());
		if (AGS_PlayerState* GSPlayerState = Cast<AGS_PlayerState>(PlayerState))
		{
			UGS_GameResultBoardUI* BoardWidget = CreateWidget<UGS_GameResultBoardUI>(this, GameResultBoardUIClass);
			if (BoardWidget)
			{
				if (GSPlayerState->CurrentPlayerRole == EPlayerRole::PR_Seeker)
				{
					BoardWidget->SetupResultBoard(GSPlayerState, PawnMappingDataAsset);
					SeekerResultBoardOverlay->AddChildToOverlay(BoardWidget);
				}
				else if (GSPlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
				{
					BoardWidget->SetupResultBoard(GSPlayerState, PawnMappingDataAsset);
					GuardianResultBoardOverlay->AddChildToOverlay(BoardWidget);
				}
			}
		}
	}
}
