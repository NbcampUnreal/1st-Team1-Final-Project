#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h" 
#include "GS_GameInstance.generated.h"

class APlayerController;

UCLASS()
class GAS_API UGS_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UGS_GameInstance();
    virtual void Init() override;

    UFUNCTION(BlueprintCallable, Category = "Network|Session")
    void HostSession(int32 MaxPlayers, FName SessionCustomName, const FString& MapName, const FString& GameModePath);

    UFUNCTION(BlueprintCallable, Category = "Network|Session")
    void FindSession(APlayerController* RequestingPlayer);

    //UFUNCTION(BlueprintCallable, Category = "Network|Session")
    void JoinSession(APlayerController* RequestingPlayer, const FOnlineSessionSearchResult& SearchResultToJoin);

protected:
    IOnlineSessionPtr SessionInterface;

    UPROPERTY()
    TWeakObjectPtr<APlayerController> PlayerSearchingSession;

    TSharedPtr<FOnlineSessionSettings> HostSessionSettings;
    FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

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

};