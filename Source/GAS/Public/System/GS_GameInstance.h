#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "GS_GameInstance.generated.h"

class APlayerController;
class FOnlineFriend;
class FOnlineSessionSearchResult;
class FUniqueNetId;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSteamFriendsListUpdated, const TArray<TSharedRef<FOnlineFriend>>& /* FriendsList */);

UCLASS()
class GAS_API UGS_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UGS_GameInstance();
    virtual void Init() override;
    virtual void Shutdown() override;


    //서버 생성, 참여

    UFUNCTION(BlueprintCallable, Category = "Network|Session")
    void GSHostSession(int32 MaxPlayers, FName SessionCustomName, const FString& MapName, const FString& GameModePath);

    UFUNCTION(BlueprintCallable, Category = "Network|Session")
    void GSFindSession(APlayerController* RequestingPlayer);

    //UFUNCTION(BlueprintCallable, Category = "Network|Session")
    void GSJoinSession(APlayerController* RequestingPlayer, const FOnlineSessionSearchResult& SearchResultToJoin);

protected:
    IOnlineSessionPtr SessionInterface;
    FOnlineSessionSearchResult SessionToJoin;

    UPROPERTY()
    TWeakObjectPtr<APlayerController> PlayerSearchingSession;

    TSharedPtr<FOnlineSessionSettings> HostSessionSettings;
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

    TSharedPtr<FOnlineSessionSearch> SessionSearchSettings;
    FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
    FDelegateHandle FindSessionsCompleteDelegateHandle;
    void OnFindSessionsComplete(bool bWasSuccessful);

    FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Network|Session Settings")
    FString DefaultLobbyMapName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Network|Session Settings")
    FString DefaultLobbyGameModePath;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Network|Session Settings")
    int32 DefaultMaxLobbyPlayers;

    // 스팀 오버레이 초대
protected:
    FString PendingConnectStringFromCmd;
public:
    FString GetAndClearPendingConnectString();

protected:
    FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegateForInvite;
    FDelegateHandle DestroySessionCompleteDelegateForInviteHandle;

    TWeakObjectPtr<APlayerController> PlayerJoiningFromInvite;
    FOnlineSessionSearchResult InviteSessionToJoinAfterDestroy;

    void LeaveCurrentSessionAndJoin(APlayerController* RequestingPlayer, const FOnlineSessionSearchResult& SearchResultToJoin);
    virtual void OnDestroySessionCompleteForInvite(FName SessionName, bool bWasSuccessful);

//    //초대
//public:
//    UFUNCTION(BlueprintCallable, Category = "Network|Friends")
//    void ReadSteamFriendsList(APlayerController* RequestingPlayer);
//
//    UFUNCTION(BlueprintCallable, Category = "Network|Friends")
//    void SendSteamSessionInvite(APlayerController* RequestingPlayer, const FUniqueNetId& FriendToInviteId);
//
//protected:
//    FOnSteamFriendsListUpdated OnSteamFriendsListUpdatedDelegate;
//    IOnlineFriendsPtr FriendsInterface;
//    TWeakObjectPtr<APlayerController> PlayerReadingFriendsList;
//
//    FOnReadFriendsListCompleteDelegate ReadFriendsCompleteDelegate;
//    FDelegateHandle ReadFriendsCompleteDelegateHandle;
//    virtual void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
//
    FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
    FDelegateHandle OnSessionUserInviteAcceptedDelegateHandle;
    virtual void OnSessionUserInviteAccepted_Impl(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);



    //타이머 넘기기
public:
    UPROPERTY(BlueprintReadWrite, Category = "Timer")
    float RemainingTime;
};