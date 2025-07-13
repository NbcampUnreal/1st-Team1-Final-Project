// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_PlayerInfoWidget.h"

#include "Character/Component/GS_StatComp.h"
#include "Character/Player/GS_Player.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/PlayerState.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
#include "System/GS_PlayerState.h"

void UGS_PlayerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsValid(OwningCharacter))
	{
		OwningCharacter = Cast<AGS_Player>(GetOwningPlayer()->GetPawn());
	}
	
	if (IsValid(OwningCharacter))
	{
		OwningCharacter->SetPlayerInfoWidget(this);
	}
}

void UGS_PlayerInfoWidget::InitializePlayerInfoWidget(AGS_Player* InPlayer)
{
	if (InPlayer)
	{
		OnCurrentHPBarChanged(InPlayer->GetStatComp());

		PlayerName->SetText(FText::FromString(InPlayer->GetPlayerState()->GetPlayerName()));
		
		ESeekerJob SeekerJob = Cast<AGS_PlayerState>(InPlayer->GetPlayerState())->CurrentSeekerJob;
		
		//[TODO] why Seeker job is not updated?
		if (InPlayer->CharacterType ==ECharacterType::Chan)
		{
			SeekerJob = ESeekerJob::Chan;
		}
		else if (InPlayer->CharacterType == ECharacterType::Ares)
		{
			SeekerJob = ESeekerJob::Ares;
		}
		else if (InPlayer->CharacterType == ECharacterType::Merci)
		{
			SeekerJob = ESeekerJob::Merci;
		}
		else if (InPlayer->CharacterType == ECharacterType::Reina)
		{
			SeekerJob = ESeekerJob::Reina;
		}
		
		const FAssetToSpawn* SpawnInfo = PawnMappingData->SeekerPawnClasses.Find(SeekerJob);
		PlayerClass->SetBrushFromTexture(SpawnInfo->ClassIconTexture);
	}
}

void UGS_PlayerInfoWidget::OnCurrentHPBarChanged(UGS_StatComp* InStatComp)
{
	HPBarWidget->SetPercent(InStatComp->GetCurrentHealth()/InStatComp->GetMaxHealth());
}
