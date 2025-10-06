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
#include "Interfaces/OnlinePresenceInterface.h"
#include "Online/OnlineSessionNames.h"


UGS_OnlineGameSubsystem::UGS_OnlineGameSubsystem()
{
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGS_OnlineGameSubsystem::OnFindSessionsComplete);
}

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

    if (!OnlineSubsystem)
	{
		OnJoinFailure.Broadcast(TEXT("Online Subsystem is invalid."));
		return;
	}

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

	// 친구의 UniqueNetId를 생성합니다.
	TSharedPtr<const FUniqueNetId> FriendNetId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(FriendIdStr);
	if (!FriendNetId.IsValid())
	{
		OnJoinFailure.Broadcast(TEXT("유효하지 않은 친구 ID입니다."));
		return;
	}

	// 로딩 화면을 표시합니다.
	if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetGameInstance()->GetFirstLocalPlayerController()))
	{
		MPC->ShowLoadingScreen();
	}

	// 세션 검색을 위한 설정 객체를 생성합니다.
	SessionSearch = MakeShared<FOnlineSessionSearch>();
	// Presence를 사용하는 세션을 검색하도록 설정합니다. (친구가 생성한 세션)
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(FName(TEXT("GAS_LOBBY")), FString(TEXT("1")), EOnlineComparisonOp::Equals);

	// 델리게이트 핸들을 등록하고 친구 세션 검색을 시작합니다.
	OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		// 검색 시작에 실패한 경우
		UE_LOG(LogTemp, Warning, TEXT("Failed to start FindSessions for friend."));
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
		OnFindSessionsComplete(false); // 실패 콜백을 직접 호출하여 정리
	}
}

void UGS_OnlineGameSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnFindSessionsComplete completed with success: %d"), bWasSuccessful);

	if (SessionInterface.IsValid())
	{
		// 사용한 델리게이트 핸들을 정리합니다.
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}

	if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() > 0)
	{
		// 검색 결과가 여러 개일 수 있지만, 친구 참가는 보통 첫 번째 결과를 사용합니다.
		const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[0];
		
		FString GameLiftSessionId;
		if (SearchResult.Session.SessionSettings.Get(FName(TEXT("GameLiftSessionId")), GameLiftSessionId) && !GameLiftSessionId.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("Found GameLiftSessionId from friend's session: %s"), *GameLiftSessionId);
			if (UGS_GameInstance* GI = Cast<UGS_GameInstance>(GetGameInstance()))
			{
				// GameInstance의 함수를 호출하여 GameLift 세션 참여를 시작합니다.
				GI->JoinGameLiftSessionByID(GameLiftSessionId);
				return; // 성공
			}
		}
	}

	// 실패 처리
	UE_LOG(LogTemp, Warning, TEXT("Could not find a valid GameLift session from friend's session search."));
	OnJoinFailure.Broadcast(TEXT("친구의 게임 세션 정보를 찾을 수 없습니다."));

	if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetGameInstance()->GetFirstLocalPlayerController()))
	{
		MPC->HideLoadingScreen();
	}
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
