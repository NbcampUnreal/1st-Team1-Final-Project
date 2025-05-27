// Copyright Epic Games, Inc. All Rights Reserved.


#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Kismet/GameplayStatics.h"

void UGS_ArcaneBoardLPS::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UGS_ArcaneBoardLPS::Deinitialize()
{
}

void UGS_ArcaneBoardLPS::ShowArcaneBoardUI()
{
}

void UGS_ArcaneBoardLPS::HideArcaneBoardUI()
{
}

bool UGS_ArcaneBoardLPS::TryCloseArcaneBoardUI()
{
	return false;
}

void UGS_ArcaneBoardLPS::UpdateStatsUI()
{
}

void UGS_ArcaneBoardLPS::ApplyBoardChanges()
{
    if (BoardManager)
    {
        BoardManager->ApplyChanges();
        UpdateCharacterStats();
        SaveBoardConfig();
    }
}

void UGS_ArcaneBoardLPS::OnBoardStatsChanged(const FGS_StatRow& NewStats)
{
    RuneSystemStats = NewStats;
    UpdateStatsUI();
}

void UGS_ArcaneBoardLPS::SaveBoardConfig()
{
}

void UGS_ArcaneBoardLPS::LoadBoardConfig()
{
}

void UGS_ArcaneBoardLPS::UpdateCharacterStats()
{
    if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
    {
        if (AGS_Character* Character = Cast<AGS_Character>(PC->GetPawn()))
        {
            if (UGS_StatComp* StatComp = Character->GetStatComp())
            {
                //StatComp->UpdateStat();
            }
        }
    }
}

void UGS_ArcaneBoardLPS::RequestServerStatsUpdate()
{
}
