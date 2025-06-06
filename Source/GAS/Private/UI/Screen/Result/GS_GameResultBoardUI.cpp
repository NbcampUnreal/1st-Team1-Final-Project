// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/Screen/Result/GS_GameResultBoardUI.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "UI/Common/CustomCommonButton.h"
#include "System/GS_PlayerState.h"
#include "System/GS_PlayerRole.h"
#include "System/GameMode/GS_ResultGM.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"


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

void UGS_GameResultBoardUI::SetupResultBoard(AGS_PlayerState* TargetPlayerState, UGS_PawnMappingDataAsset* PawnMappingDataAsset)
{
    if (!TargetPlayerState || !PawnMappingDataAsset || !Image_PlayerPortrait)
    {
        return;
    }

    EPlayerRole PlayerRole = TargetPlayerState->CurrentPlayerRole;
    
    //플레이어 아이디
    Text_PlayerID->SetText(FText::FromString(TargetPlayerState->GetPlayerName()));

    //캐릭터 이미지
    UTexture2D* FoundTexture = nullptr;
    if (PlayerRole == EPlayerRole::PR_Seeker)
    {
        ESeekerJob SeekerJob = TargetPlayerState->CurrentSeekerJob;
        if (const FAssetToSpawn* FoundAsset = PawnMappingDataAsset->SeekerPawnClasses.Find(SeekerJob))
        {
            FoundTexture = FoundAsset->AvatarTexture;
        }
    }
    else if (PlayerRole == EPlayerRole::PR_Guardian)
    {
        EGuardianJob GuardianJob = TargetPlayerState->CurrentGuardianJob;
        if (const FAssetToSpawn* FoundAsset = PawnMappingDataAsset->GuardianPawnClasses.Find(GuardianJob))
        {
            FoundTexture = FoundAsset->AvatarTexture;
        }
    }
    if (FoundTexture)
    {
        Image_PlayerPortrait->SetBrushFromTexture(FoundTexture);
    }

    //승패
    EGameResult Result = TargetPlayerState->CurrentGameResult;

	if (Result == EGameResult::GR_SeekersWon)
	{
		Text_Victory->SetText(FText::FromString(TEXT("Victory")));
	}
	else if (Result == EGameResult::GR_SeekersLost)
	{
		Text_Victory->SetText(FText::FromString(TEXT("Defeat")));
	}
	else
	{
		Text_Victory->SetText(FText::FromString(TEXT("진행 중")));
	}

    //플레이어 생사
    if (TargetPlayerState->bIsAlive)
    {
		Image_PlayerStatus->SetBrushFromTexture(AliveTexture);
    }
    else
    {
		Image_PlayerStatus->SetBrushFromTexture(DeadTexture);
    }

	//레벨
	Text_Level->SetText(FText::FromString(FString::Printf(TEXT("Lv.%d"), TargetPlayerState->GetLevel())));
}
