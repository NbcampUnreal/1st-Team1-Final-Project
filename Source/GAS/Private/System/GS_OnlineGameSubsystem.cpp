#include "System/GS_OnlineGameSubsystem.h"
#include "System/GS_GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "System/PlayerController/GS_MainMenuPC.h"


void UGS_OnlineGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		FriendsInterface = OnlineSubsystem->GetFriendsInterface();
	}
}

void UGS_OnlineGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGS_OnlineGameSubsystem::ReadFriendsList()
{
	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineGameSubsystem: ReadFriendsList failed - FriendsInterface is not valid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineGameSubsystem: ReadFriendsList failed - LocalPlayer is not valid."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: Calling ReadFriendsList for user %d."), LocalPlayer->GetControllerId());
	FOnReadFriendsListComplete Delegate = FOnReadFriendsListComplete::CreateUObject(this, &UGS_OnlineGameSubsystem::OnReadFriendsListComplete_Callback);
	FriendsInterface->ReadFriendsList(LocalPlayer->GetControllerId(), EFriendsLists::ToString(EFriendsLists::Default), Delegate);
}

void UGS_OnlineGameSubsystem::JoinFriend(const FString& FriendIdStr)
{
	UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: Attempting to join friend with ID: %s"), *FriendIdStr);
	if (!SessionInterface.IsValid())
	{
		OnJoinFailure.Broadcast(TEXT("Session Interface is invalid."));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if (!LocalPlayer)
	{
		OnJoinFailure.Broadcast(TEXT("Local Player is invalid."));
		return;
	}
	const TSharedPtr<const FUniqueNetId> LocalPlayerNetIdPtr = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!LocalPlayerNetIdPtr.IsValid())
	{
		OnJoinFailure.Broadcast(TEXT("로컬 플레이어 ID가 유효하지 않습니다."));
		return;
	}
	const TSharedRef<const FUniqueNetId> LocalPlayerNetId = LocalPlayerNetIdPtr.ToSharedRef();

	FriendToJoinId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(FriendIdStr);
	if (!FriendToJoinId.IsValid())
	{
		OnJoinFailure.Broadcast(TEXT("유효하지 않은 친구 ID입니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: Calling FindFriendSession for friend ID: %s"), *FriendIdStr);
	FindFriendSessionCompleteDelegateHandle = SessionInterface->OnFindFriendSessionCompleteDelegates[LocalPlayer->GetControllerId()].AddUObject(this, &UGS_OnlineGameSubsystem::OnFindFriendSessionComplete);
	SessionInterface->FindFriendSession(LocalPlayer->GetControllerId(), *FriendToJoinId);
}

void UGS_OnlineGameSubsystem::OnReadFriendsListComplete_Callback(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: OnReadFriendsListComplete_Callback - Success: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));
	TArray<TSharedRef<FOnlineFriend>> FriendsList;
	if (bWasSuccessful && FriendsInterface.IsValid())
	{
		FriendsInterface->GetFriendsList(LocalUserNum, ListName, FriendsList);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to read friends list: %s"), *ErrorStr);
	}
	OnFriendsReadComplete.Broadcast(FriendsList);
}

void UGS_OnlineGameSubsystem::OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: OnFindFriendSessionComplete - Success: %s, Results found: %d"), bWasSuccessful ? TEXT("true") : TEXT("false"), SearchResults.Num());

	if (FindFriendSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnFindFriendSessionCompleteDelegates(LocalUserNum, this);
		FindFriendSessionCompleteDelegateHandle.Reset();
	}

	if (bWasSuccessful && !SearchResults.IsEmpty())
	{
		const FOnlineSessionSearchResult& FoundSession = SearchResults[0];
		UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: Found friend's session. SessionInfo: %s"), *FoundSession.GetSessionIdStr());

		if (FoundSession.Session.SessionInfo.IsValid())
		{
			if (UGS_GameInstance* GI = Cast<UGS_GameInstance>(GetGameInstance()))
			{
				if (APlayerController* PC = GI->GetFirstLocalPlayerController())
				{
					if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(PC))
					{
						MPC->ShowLoadingScreen();
					}
					GI->bJoiningFromInvite = true;
					GI->LeaveCurrentSessionAndJoin(PC, FoundSession);
				}
				else
				{
					OnJoinFailure.Broadcast(TEXT("플레이어 컨트롤러를 찾을 수 없습니다."));
					FriendToJoinId.Reset();
				}
			}
			else
			{
				JoinSessionCompleteDelegateHandle = SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UGS_OnlineGameSubsystem::OnJoinSessionComplete);
				SessionInterface->JoinSession(LocalUserNum, NAME_GameSession, FoundSession);
			}
			return;
		}
	}

	OnJoinFailure.Broadcast(TEXT("친구의 세션을 찾을 수 없습니다."));
	FriendToJoinId.Reset();
}

void UGS_OnlineGameSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Log, TEXT("OnlineGameSubsystem: OnJoinSessionComplete - SessionName: %s, Result: %s"), *SessionName.ToString(), LexToString(Result));

	if (JoinSessionCompleteDelegateHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
		JoinSessionCompleteDelegateHandle.Reset();
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
			{
				OnJoinSuccess.Broadcast();
				PC->ClientTravel(ConnectString, TRAVEL_Absolute);
				FriendToJoinId.Reset();
				return;
			}
		}
	}

	FriendToJoinId.Reset();
	OnJoinFailure.Broadcast(FString::Printf(TEXT("참가 실패: %s"), LexToString(Result)));
}