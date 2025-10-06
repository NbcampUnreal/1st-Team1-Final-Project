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
	UGS_OnlineGameSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void ReadFriendsList();
	void JoinFriend(const FString& FriendIdStr);

	FOnFriendsReadComplete OnFriendsReadComplete;
	FOnJoinSuccess OnJoinSuccess;
	FOnJoinFailure OnJoinFailure;

	/** 세션 검색이 완료되었을 때 호출될 델리게이트 */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	/** OnFindSessionsCompleteDelegate의 델리게이트 핸들 */
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	/** 세션 검색이 완료되었을 때 호출될 콜백 함수 */
	void OnFindSessionsComplete(bool bWasSuccessful);
	
private:
	void OnReadFriendsListComplete_Callback(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	IOnlineSubsystem* OnlineSubsystem;
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr FriendsInterface;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};