#include "System/GS_GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h" //SETTING_GAMEMODE 이런 거 쓸라면 필요. 앞으로 까먹지 말기
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

UGS_GameInstance::UGS_GameInstance()
    : DefaultLobbyMapName(TEXT("/Game/Maps/CustomLobbyLevel"))
	, DefaultLobbyGameModePath(TEXT("/Game/System/BP_GS_CustomLobbyGM.BP_GS_CustomLobbyGM_C"))
	, DefaultMaxLobbyPlayers(5)
{
    RemainingTime = 900.f;
}

void UGS_GameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Init() CALLED."));

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {

        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Online Subsystem '%s' found."), *Subsystem->GetSubsystemName().ToString());

        FString ConnectString;
        // "+connect IP:PORT" 형태의 명령줄 인자 확인
        if (FParse::Value(FCommandLine::Get(), TEXT("+connect="), ConnectString) || FParse::Value(FCommandLine::Get(), TEXT("connect="), ConnectString))
        {
            ConnectString = ConnectString.TrimStartAndEnd();
            if (!ConnectString.IsEmpty() && ConnectString.Contains(TEXT(":"))) // 간단한 IP:Port 형식 검증
            {
                UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Found command line connect string: '%s'"), *ConnectString);
                PendingConnectStringFromCmd = ConnectString;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance: Found 'connect' cmd arg, but it seems invalid: '%s'"), *ConnectString);
            }
        }

        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: SessionInterface is VALID. Binding delegates."));
            CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::OnCreateSessionComplete);
            FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UGS_GameInstance::OnFindSessionsComplete);
            JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::OnJoinSessionComplete);
            DestroySessionCompleteDelegateForInvite = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::OnDestroySessionCompleteForInvite);
            OnSessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UGS_GameInstance::OnSessionUserInviteAccepted_Impl);
            if (OnSessionUserInviteAcceptedDelegate.IsBound()) //FOnSessionUserInviteAcceptedDelegate는 게임 인스턴스 초기화 시점부터 계속 리스닝해야 하므로 Init()에서 핸들까지 등록
            {
                OnSessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);
                if (OnSessionUserInviteAcceptedDelegateHandle.IsValid())
                {
                    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: OnSessionUserInviteAcceptedDelegate BOUND and Handle REGISTERED."));
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance: Failed to REGISTER OnSessionUserInviteAcceptedDelegate Handle."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance: OnSessionUserInviteAcceptedDelegate FAILED TO BIND. Cannot register handle."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance: SessionInterface is NOT valid in Init()."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance: Online Subsystem NOT found in Init()."));
    }

    if (IsDedicatedServerInstance())
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Init() - This is a Dedicated Server Instance. Attempting to host a session."));
        GSHostSession(DefaultMaxLobbyPlayers, NAME_GameSession, DefaultLobbyMapName, DefaultLobbyGameModePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Init() - Not a Dedicated Server Instance."));
    }
}

FString UGS_GameInstance::GetAndClearPendingConnectString()
{
	FString TempConnectString = PendingConnectStringFromCmd;
	PendingConnectStringFromCmd.Empty();
	return TempConnectString;
}

void UGS_GameInstance::LeaveCurrentSessionAndJoin(APlayerController* RequestingPlayer, const FOnlineSessionSearchResult& SearchResultToJoin)
{
    if (!RequestingPlayer || !SearchResultToJoin.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - Invalid parameters."));
        return;
    }

    PlayerJoiningFromInvite = RequestingPlayer;
    InviteSessionToJoinAfterDestroy = SearchResultToJoin;

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - SessionInterface is invalid. Attempting to join directly."));
        GSJoinSession(RequestingPlayer, SearchResultToJoin); // SessionInterface가 없으면 직접 Join 시도
        return;
    }

    // NAME_GameSession은 호스트/참여 시 사용한 세션 이름이어야 함
    FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (CurrentSession != nullptr && CurrentSession->SessionState != EOnlineSessionState::NoSession) // NoSession이 아니거나, Pending 등 다른 상태일 때도 파괴 시도
    {
        if (DestroySessionCompleteDelegateForInviteHandle.IsValid()) // 이미 진행 중인 작업이 있다면? 보통은 없어야 함.
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInviteHandle);
            DestroySessionCompleteDelegateForInviteHandle.Reset();
            UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - Cleared existing DestroySessionCompleteDelegateForInviteHandle."));
        }
        DestroySessionCompleteDelegateForInviteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInvite);

        if (!SessionInterface->DestroySession(NAME_GameSession))
        {
            UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - DestroySession call failed immediately. Clearing delegate and attempting to join directly."));
            if (DestroySessionCompleteDelegateForInviteHandle.IsValid())
            {
                SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInviteHandle);
                DestroySessionCompleteDelegateForInviteHandle.Reset();
            }
            GSJoinSession(PlayerJoiningFromInvite.Get(), InviteSessionToJoinAfterDestroy);
        }
        // 성공적으로 DestroySession 호출 시, OnDestroySessionCompleteForInvite 콜백 대기
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - Player is not in a known session or session state is NoSession. Joining invite session directly."));
        GSJoinSession(RequestingPlayer, SearchResultToJoin);
    }
}

void UGS_GameInstance::OnDestroySessionCompleteForInvite(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnDestroySessionCompleteForInvite - Session '%s' destruction: %s. Proceeding to join invite session."), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (DestroySessionCompleteDelegateForInviteHandle.IsValid() && SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInviteHandle);
        DestroySessionCompleteDelegateForInviteHandle.Reset();
    }

    APlayerController* PC = PlayerJoiningFromInvite.Get();
    if (PC && InviteSessionToJoinAfterDestroy.IsValid())
    {
        GSJoinSession(PC, InviteSessionToJoinAfterDestroy);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnDestroySessionCompleteForInvite - PlayerController or InviteSessionToJoinAfterDestroy became invalid after session destruction. Cannot join."));
    }
    PlayerJoiningFromInvite = nullptr;
}

void UGS_GameInstance::OnSessionUserInviteAccepted_Impl(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnSessionUserInviteAccepted_Impl - Invite accepted by LocalUserNum: %d, Success: %s"), ControllerId, bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (bWasSuccessful && InviteResult.IsValid())
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), ControllerId); // ControllerId 사용
        if (PC)
        {
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnSessionUserInviteAccepted_Impl - Player %s accepting invite. Attempting to leave current session (if any) and join."), *PC->GetName());
            LeaveCurrentSessionAndJoin(PC, InviteResult); // JoinSession 대신 이 함수 호출
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::OnSessionUserInviteAccepted_Impl - Could not get PlayerController for ControllerId: %d"), ControllerId);
        }
    }
    else
    {
        // ... (실패 로그) ...
    }
}

void UGS_GameInstance::Shutdown()
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Shutdown() CALLED."));
    // Clear all delegate handles
    if (SessionInterface.IsValid())
    {
        if (CreateSessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
            CreateSessionCompleteDelegateHandle.Reset();
        }
        if (FindSessionsCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
            FindSessionsCompleteDelegateHandle.Reset();
        }
        if (JoinSessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
            JoinSessionCompleteDelegateHandle.Reset();
        }
		if (DestroySessionCompleteDelegateForInviteHandle.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInviteHandle);
            DestroySessionCompleteDelegateForInviteHandle.Reset();
		}
        if (OnSessionUserInviteAcceptedDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegateHandle);
            OnSessionUserInviteAcceptedDelegateHandle.Reset();
        }
    }
    Super::Shutdown();
}

void UGS_GameInstance::GSHostSession(int32 MaxPlayers, FName SessionCustomName, const FString& MapName, const FString& GameModePath)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: HostSession() CALLED - MaxPlayers: %d, SessionName: %s, Map: %s, GameMode: %s"), MaxPlayers, *SessionCustomName.ToString(), *MapName, *GameModePath);
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::HostSession - Session interface is not valid. Cannot Host Sesison."));
        return;
    }

    HostSessionSettings = MakeShareable(new FOnlineSessionSettings());
    HostSessionSettings->NumPublicConnections = 5; //이거 나중에 1로 바꾸기
    HostSessionSettings->NumPrivateConnections = MaxPlayers - HostSessionSettings->NumPublicConnections;
    HostSessionSettings->bShouldAdvertise = true;
    HostSessionSettings->bIsLANMatch = false;
    HostSessionSettings->bUsesPresence = false; // 스팀데디에서 이거 반드시 꺼야됨
    HostSessionSettings->bUseLobbiesIfAvailable = false;  //bUsesPresence 값이랑 동일해야함
    HostSessionSettings->bAllowJoinViaPresence = true;
    HostSessionSettings->bAllowJoinInProgress = true;
    HostSessionSettings->bAllowInvites = true;
    HostSessionSettings->bIsDedicated = IsDedicatedServerInstance();
    HostSessionSettings->bAllowInvites = true;
    HostSessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    HostSessionSettings->Set(SETTING_GAMEMODE, GameModePath, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    //HostSessionSettings->Set(FName(TEXT("IINGS")), FString(TEXT("SpartaFinal")), EOnlineDataAdvertisementType::ViaOnlineService);
	HostSessionSettings->Set(SEARCH_KEYWORDS, FString("IINGSSpartaFinal"), EOnlineDataAdvertisementType::ViaOnlineService);

    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::HostSession - SessionSettings: PublicSlots=%d, PrivateSlots=%d, bIsDedicated=%s"),
        HostSessionSettings->NumPublicConnections, HostSessionSettings->NumPrivateConnections, HostSessionSettings->bIsDedicated ? TEXT("true") : TEXT("false"));

    if (!CreateSessionCompleteDelegateHandle.IsValid())
    {
        CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::HostSession - CreateSessionCompleteDelegateHandle BOUND."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::HostSession - CreateSessionCompleteDelegateHandle was ALREADY VALID. This might indicate a previous operation wasn't cleaned up properly."));
    }

    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::HostSession - Attempting to create session: %s"), *SessionCustomName.ToString());
    if (!SessionInterface->CreateSession(0, SessionCustomName, *HostSessionSettings))
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::HostSession - Call to CreateSession failed immediately for session: %s."), *SessionCustomName.ToString());
        if (CreateSessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
            CreateSessionCompleteDelegateHandle.Reset();
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::HostSession - CreateSessionCompleteDelegateHandle CLEARED due to immediate CreateSession failure."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::HostSession - CreateSession call initiated for session: %s. Waiting for OnCreateSessionComplete callback."), *SessionCustomName.ToString());
    }
}

void UGS_GameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: OnCreateSessionComplete() CALLED - SessionName: %s, Success: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
    if (SessionInterface.IsValid() && CreateSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
        CreateSessionCompleteDelegateHandle.Reset();
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnCreateSessionComplete - Delegate handle CLEARED."));
    }

    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Session '%s' created successfully on the backend."), *SessionName.ToString());
		if (SessionInterface.IsValid())
		{
			SessionInterface->StartSession(SessionName);
			UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Session '%s' started successfully."), *SessionName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance: SessionInterface is not valid when starting session '%s'."), *SessionName.ToString());
		}
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance: Failed to create session '%s' on the backend."), *SessionName.ToString());
    }
}

void UGS_GameInstance::GSFindSession(APlayerController* RequestingPlayer)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: FindSession() CALLED by Player: %s"), RequestingPlayer ? *RequestingPlayer->GetName() : TEXT("Unknown"));
    if (!RequestingPlayer || !SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::FindSession - Invalid player controller or session interface. Aborting."));
        return;
    }
    PlayerSearchingSession = RequestingPlayer;

    SessionSearchSettings = MakeShareable(new FOnlineSessionSearch());
    SessionSearchSettings->MaxSearchResults = 7777;
    SessionSearchSettings->bIsLanQuery = false;
    SessionSearchSettings->QuerySettings.SearchParams.Remove(SEARCH_PRESENCE);
	//SessionSearchSettings->QuerySettings.Set(FName(TEXT("IINGS")), FString(TEXT("SpartaFinal")), EOnlineComparisonOp::Equals);
	SessionSearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, FString("IINGSSpartaFinal"), EOnlineComparisonOp::Equals);
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::FindSession - SearchSettings: MaxResults=%d, LANQuery=%s, PresenceQuery=%s"),
        SessionSearchSettings->MaxSearchResults, SessionSearchSettings->bIsLanQuery ? TEXT("true") : TEXT("false"), TEXT("true"));


    if (!FindSessionsCompleteDelegateHandle.IsValid())
    {
        FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::FindSession - FindSessionsCompleteDelegateHandle BOUND."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::FindSession - FindSessionsCompleteDelegateHandle was ALREADY VALID."));
    }

    ULocalPlayer* LocalPlayer = RequestingPlayer->GetLocalPlayer();
    int32 ControllerIdToSearch = LocalPlayer ? LocalPlayer->GetControllerId() : 0;

    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::FindSession - Player %s (Controller ID: %d) initiating FindSessions..."), *RequestingPlayer->GetName(), ControllerIdToSearch);
    if (!SessionInterface->FindSessions(ControllerIdToSearch, SessionSearchSettings.ToSharedRef()))
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::FindSession - Call to FindSessions failed immediately."));
        if (FindSessionsCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
            FindSessionsCompleteDelegateHandle.Reset();
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::FindSession - FindSessionsCompleteDelegateHandle CLEARED due to immediate FindSessions failure."));
        }
        PlayerSearchingSession = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::FindSession - FindSessions call initiated. Waiting for OnFindSessionsComplete callback."));
    }
}

void UGS_GameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: OnFindSessionsComplete() CALLED - Success: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (SessionInterface.IsValid() && FindSessionsCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
        FindSessionsCompleteDelegateHandle.Reset();
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnFindSessionsComplete - Delegate handle CLEARED."));
    }

    APlayerController* PC = PlayerSearchingSession.Get();

    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnFindSessionsComplete - PlayerSearchingSession (PlayerController) is no longer valid. Aborting."));
        return;
    }

    if (bWasSuccessful && SessionSearchSettings.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnFindSessionsComplete - Found %d sessions."), SessionSearchSettings->SearchResults.Num());
        if (SessionSearchSettings->SearchResults.Num() > 0)
        {
            bool bFoundSuitableSession = false;
            for (const FOnlineSessionSearchResult& SearchResult : SessionSearchSettings->SearchResults)
            {
                if (SearchResult.IsValid() && SearchResult.Session.SessionSettings.bIsDedicated && SearchResult.Session.NumOpenPublicConnections > 0)
                {
                    FString CustomKeyCheck;
                    bool bCustomKeyFound = SearchResult.Session.SessionSettings.Get(SEARCH_KEYWORDS, CustomKeyCheck);

                    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnFindSessionsComplete - Checking SessionID: %s, OwningUser: %s, OpenPublic: %d, CustomKeyFound: %s, CustomKeyValue: %s"),
                        *SearchResult.GetSessionIdStr(),
                        *SearchResult.Session.OwningUserName,
                        SearchResult.Session.NumOpenPublicConnections,
                        bCustomKeyFound ? TEXT("true") : TEXT("false"),
                        bCustomKeyFound ? *CustomKeyCheck : TEXT("N/A"));

                    if (bCustomKeyFound && CustomKeyCheck == TEXT("IINGSSpartaFinal"))
                    {
                        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnFindSessionsComplete - YOUR DEDICATED session with matching custom key found! Attempting to join."));

                        SessionToJoin = SearchResult;
                        GSJoinSession(PC, SearchResult);
                        bFoundSuitableSession = true;
                        return;
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnFindSessionsComplete - Session %s is NOT suitable (IsValid: %s, IsDedicated: %s, OpenPublicSlots: %d)."),
                        *SearchResult.GetSessionIdStr(),
                        SearchResult.IsValid() ? TEXT("true") : TEXT("false"),
                        SearchResult.IsValid() && SearchResult.Session.SessionSettings.bIsDedicated ? TEXT("true") : TEXT("false"),
                        SearchResult.IsValid() ? SearchResult.Session.NumOpenPublicConnections : -1
                    );
                }
            }
            if (!bFoundSuitableSession)
            {
                UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnFindSessionsComplete - No suitable DEDICATED sessions found after filtering all %d results."), SessionSearchSettings->SearchResults.Num());
                PlayerSearchingSession = nullptr;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnFindSessionsComplete - No sessions found in search results (Num: 0)."));
            PlayerSearchingSession = nullptr;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnFindSessionsComplete - FindSessions bWasSuccessful is false or SessionSearchSettings is invalid."));
        PlayerSearchingSession = nullptr;
    }
}

void UGS_GameInstance::GSJoinSession(APlayerController* RequestingPlayer, const FOnlineSessionSearchResult& SearchResultToJoin)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::JoinSession() CALLED by Player: %s for SessionID: %s"),
        RequestingPlayer ? *RequestingPlayer->GetName() : TEXT("Unknown"),
        *SearchResultToJoin.GetSessionIdStr()
    );

    if (!RequestingPlayer || !SessionInterface.IsValid() || !SearchResultToJoin.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::JoinSession - Invalid parameters. RequestingPlayer: %s, SessionInterface: %s, SearchResultToJoin: %s. Aborting."),
            RequestingPlayer ? TEXT("Valid") : TEXT("NULL"),
            SessionInterface.IsValid() ? TEXT("Valid") : TEXT("NULL"),
            SearchResultToJoin.IsValid() ? TEXT("Valid") : TEXT("NULL")
        );
        PlayerSearchingSession = nullptr;
        return;
    }

    PlayerSearchingSession = RequestingPlayer;

    if (JoinSessionCompleteDelegateHandle.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::JoinSession - Clearing existing JoinSessionCompleteDelegateHandle."));
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        JoinSessionCompleteDelegateHandle.Reset();
    }
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::JoinSession - JoinSessionCompleteDelegateHandle BOUND."));

    ULocalPlayer* LocalPlayer = RequestingPlayer->GetLocalPlayer();
    int32 ControllerIdToJoin = LocalPlayer ? LocalPlayer->GetControllerId() : 0;

    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::JoinSession - Attempting to join session: %s with Controller ID: %d"), *SearchResultToJoin.GetSessionIdStr(), ControllerIdToJoin);
    if (!SessionInterface->JoinSession(ControllerIdToJoin, NAME_GameSession, SearchResultToJoin))
    {
        UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::JoinSession - Call to JoinSession failed immediately for session: %s"), *SearchResultToJoin.GetSessionIdStr());
        if (JoinSessionCompleteDelegateHandle.IsValid())
        {
            SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
            JoinSessionCompleteDelegateHandle.Reset();
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::JoinSession - JoinSessionCompleteDelegateHandle CLEARED due to immediate JoinSession failure."));
        }
        PlayerSearchingSession = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::JoinSession - JoinSession call initiated for session: %s. Waiting for OnJoinSessionComplete callback."), *SearchResultToJoin.GetSessionIdStr());
    }
}

void UGS_GameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnJoinSessionComplete() CALLED - SessionName: %s"), *SessionName.ToString());

    APlayerController* PC = PlayerSearchingSession.Get();
    PlayerSearchingSession = nullptr;

    if (SessionInterface.IsValid() && JoinSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        JoinSessionCompleteDelegateHandle.Reset();
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnJoinSessionComplete - Delegate handle CLEARED."));
    }

    if (Result == EOnJoinSessionCompleteResult::Success && PC)
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnJoinSessionComplete - Join successful for PC: %s! Attempting to get connect string for session: %s."), *PC->GetName(), *SessionName.ToString());
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnJoinSessionComplete - Resolved ConnectString: %s"), *ConnectString);
            FString MapToTravel = DefaultLobbyMapName; // 기본값
            FString GameModeToTravel = DefaultLobbyGameModePath; // 기본값

            if (SessionToJoin.IsValid()) // 저장된 세션 정보 사용
            {
                FString MapNameFromSession;
                if (SessionToJoin.Session.SessionSettings.Get(SETTING_MAPNAME, MapNameFromSession) && !MapNameFromSession.IsEmpty())
                {
                    MapToTravel = MapNameFromSession;
                }
                FString GameModeFromSession;
                if (SessionToJoin.Session.SessionSettings.Get(SETTING_GAMEMODE, GameModeFromSession) && !GameModeFromSession.IsEmpty())
                {
                    GameModeToTravel = GameModeFromSession;
                }
                UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnJoinSessionComplete - Using Map: %s, GameMode: %s from actual joined session settings."), *MapToTravel, *GameModeToTravel);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnJoinSessionComplete - SessionToJoin was not valid, using default Map/GameMode for travel."));
            }

            FString TravelURL;
            if (!ConnectString.Contains(TEXT("/Game/Maps"))) {
                TravelURL = ConnectString + MapToTravel + TEXT("?game=") + GameModeToTravel;
            }
            else {
                TravelURL = ConnectString + TEXT("?game=") + GameModeToTravel;
            }

            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnJoinSessionComplete - Traveling PC %s to: %s"), *PC->GetName(), *TravelURL);
            PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnJoinSessionComplete - Could not get resolved connect string for session: %s"), *SessionName.ToString());
        }
    }
    else
    {
        FString ResultStr;
        switch (Result)
        {
        case EOnJoinSessionCompleteResult::Success: ResultStr = TEXT("Success (but PC was invalid or became invalid)"); break;
        case EOnJoinSessionCompleteResult::SessionIsFull: ResultStr = TEXT("SessionIsFull"); break;
        case EOnJoinSessionCompleteResult::SessionDoesNotExist: ResultStr = TEXT("SessionDoesNotExist"); break;
        case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: ResultStr = TEXT("CouldNotRetrieveAddress"); break;
        case EOnJoinSessionCompleteResult::AlreadyInSession: ResultStr = TEXT("AlreadyInSession"); break;
        default: ResultStr = TEXT("UnknownError"); break;
        }
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnJoinSessionComplete - JoinSession for '%s' failed. Result: %s. PC Valid: %s"),
            *SessionName.ToString(), *ResultStr, PC ? TEXT("true") : TEXT("false"));
    }
}
