#include "System/GameState/GS_CustomLobbyGS.h"
#include "Net/UnrealNetwork.h"
#include "System/GS_PlayerState.h"

void AGS_CustomLobbyGS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_CustomLobbyGS, PlayerList);
}

void AGS_CustomLobbyGS::AddPlayerToList(AGS_PlayerState* PlayerState, int32 SlotIndex)
{
	if (!HasAuthority() || !PlayerState) return;

	// 중복 추가 방지
	if (PlayerList.ContainsByPredicate([PlayerState](const FLobbyPlayerInfo& Info) { return Info.PlayerState == PlayerState; }))
	{
		return;
	}

	FLobbyPlayerInfo NewPlayerInfo;
	NewPlayerInfo.PlayerState = PlayerState;
	NewPlayerInfo.PlayerName = PlayerState->GetPlayerName();
	NewPlayerInfo.PlayerRole = PlayerState->CurrentPlayerRole;
	NewPlayerInfo.SeekerJob = PlayerState->CurrentSeekerJob;
	NewPlayerInfo.GuardianJob = PlayerState->CurrentGuardianJob;
	NewPlayerInfo.SlotIndex = SlotIndex;
	NewPlayerInfo.bIsReady = PlayerState->bIsReady;

	PlayerList.Add(NewPlayerInfo);

	// 로컬(서버)에서 즉시 델리게이트 호출
	OnRep_PlayerList();
}

void AGS_CustomLobbyGS::RemovePlayerFromList(AGS_PlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState) return;

	PlayerList.RemoveAll([PlayerState](const FLobbyPlayerInfo& Info) {
		return Info.PlayerState == PlayerState;
	});

	OnRep_PlayerList();
}

void AGS_CustomLobbyGS::UpdatePlayerInList(AGS_PlayerState* PlayerState, int32 NewSlotIndex)
{
	if (!HasAuthority() || !PlayerState) return;

	for (FLobbyPlayerInfo& PlayerInfo : PlayerList)
	{
		if (PlayerInfo.PlayerState == PlayerState)
		{
			PlayerInfo.PlayerRole = PlayerState->CurrentPlayerRole;
			PlayerInfo.SeekerJob = PlayerState->CurrentSeekerJob;
			PlayerInfo.GuardianJob = PlayerState->CurrentGuardianJob;
			PlayerInfo.bIsReady = PlayerState->bIsReady;
			if (NewSlotIndex != -1)
			{
				PlayerInfo.SlotIndex = NewSlotIndex;
			}
			break;
		}
	}

	OnRep_PlayerList();
}

void AGS_CustomLobbyGS::OnRep_PlayerList()
{
	OnPlayerListUpdated.Broadcast();
}

