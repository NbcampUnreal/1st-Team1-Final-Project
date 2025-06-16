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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerCountChangedDelegate);

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

    // 세션 나가기
public:
	UFUNCTION(BlueprintCallable, Category = "Network|Session")
	void GSLeaveSession(APlayerController* RequestingPlayer);

protected:
    FOnDestroySessionCompleteDelegate LeaveSessionCompleteDelegate;
    FDelegateHandle LeaveSessionCompleteDelegateHandle;
    void OnLeaveSessionComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Network|Session Settings")
    FString MainMenuMapPath;

    // 스팀 오버레이 초대
protected:
    FString PendingConnectStringFromCmd;
public:
    FString GetAndClearPendingConnectString();

    bool bJoiningFromInvite = false;

protected:
    FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegateForInvite;
    FDelegateHandle DestroySessionCompleteDelegateForInviteHandle;

    TWeakObjectPtr<APlayerController> PlayerJoiningFromInvite;
    FOnlineSessionSearchResult InviteSessionToJoinAfterDestroy;

    void LeaveCurrentSessionAndJoin(APlayerController* RequestingPlayer, const FOnlineSessionSearchResult& SearchResultToJoin);
    virtual void OnDestroySessionCompleteForInvite(FName SessionName, bool bWasSuccessful);

    FOnSessionUserInviteAcceptedDelegate OnSessionUserInviteAcceptedDelegate;
    FDelegateHandle OnSessionUserInviteAcceptedDelegateHandle;
    virtual void OnSessionUserInviteAccepted_Impl(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);

    virtual void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

    FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegateForCleanup;
    FDelegateHandle OnDestroySessionCompleteDelegateHandleForCleanup;
    void OnDestroySessionCompleteForCleanup(FName SessionName, bool bWasSuccessful);

    //세션 생명주기 관리
public:
    UPROPERTY(BlueprintAssignable, Category = "Session")
    FOnPlayerCountChangedDelegate OnPlayerCountChanged;

private:
    UFUNCTION()
    void HandlePlayerCountChanged();

    //타이머 넘기기
public:
    UPROPERTY(BlueprintReadWrite, Category = "Timer")
    float RemainingTime;

    //플레이어 옵션 세팅
public:
    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetMouseSensitivity() const;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseSensitivity(float NewSensitivity);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SaveSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void LoadSettings();

 private:
     UPROPERTY(BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
     float MouseSensitivity;

     UPROPERTY(BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
     float MinSensitivity;

     UPROPERTY(BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
     float MaxSensitivity;
};