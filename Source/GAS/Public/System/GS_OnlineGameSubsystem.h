#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystem.h"
#include "GS_OnlineGameSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFriendsReadComplete, const TArray<TSharedRef<FOnlineFriend>>& /* FriendsList */);
DECLARE_MULTICAST_DELEGATE(FOnJoinSuccess);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinFailure, const FString& /* Reason */);

UCLASS()
class GAS_API UGS_OnlineGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void ReadFriendsList();
	void JoinFriend(const FString& FriendIdStr);

	FOnFriendsReadComplete OnFriendsReadComplete;
	FOnJoinSuccess OnJoinSuccess;
	FOnJoinFailure OnJoinFailure;

private:
	void OnReadFriendsListComplete_Callback(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	void OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSubsystem* OnlineSubsystem;
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr FriendsInterface;

	FDelegateHandle FindFriendSessionCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	TSharedPtr<const FUniqueNetId> FriendToJoinId;
};