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
#include "System/Save/GS_OptionSettinsSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "System/PlayerController/GS_MainMenuPC.h"

DEFINE_LOG_CATEGORY(GameServerLog);

UGS_GameInstance::UGS_GameInstance()
    : DefaultLobbyMapName(TEXT("/Game/Maps/CustomLobbyLevel"))
    , MainMenuMapPath(TEXT("/Game/Maps/MainLevel"))
	, DefaultLobbyGameModePath(TEXT("/Game/System/BP_GS_CustomLobbyGM.BP_GS_CustomLobbyGM_C"))
	, DefaultMaxLobbyPlayers(5)
{
    RemainingTime = 900.f;

    MouseSensitivity = 1.0f;
    MinSensitivity = 0.1f;
    MaxSensitivity = 10.0f;
}

void UGS_GameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Init() CALLED."));

    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UGS_GameInstance::HandleNetworkFailure);
    }

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
            OnDestroySessionCompleteDelegateForCleanup = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::OnDestroySessionCompleteForCleanup);
            LeaveSessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UGS_GameInstance::OnLeaveSessionComplete);
            OnPlayerCountChanged.AddDynamic(this, &UGS_GameInstance::HandlePlayerCountChanged);
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

#if WITH_GAMELIFT
        // GameLift 빌드인 경우: GameLift SDK를 초기화하고 GameLift의 지시를 기다린다.
        InitGameLift();
#else
        // 일반 데디 서버 빌드인 경우: 직접 스팀 세션을 호스팅한다.
        GSHostSession(DefaultMaxLobbyPlayers, NAME_GameSession, DefaultLobbyMapName, DefaultLobbyGameModePath);
#endif
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Init() - Not a Dedicated Server Instance."));
    }

    // 저장된 옵션 세팅 로드
    LoadSettings();
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
        bJoiningFromInvite = true;
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), ControllerId); // ControllerId 사용
        if (PC)
        {
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnSessionUserInviteAccepted_Impl - Player %s accepting invite. Attempting to leave current session (if any) and join."), *PC->GetName());
            if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(PC))
            {
                MPC->ShowLoadingScreen();
            }
            LeaveCurrentSessionAndJoin(PC, InviteResult);
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

void UGS_GameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    UE_LOG(LogTemp, Error, TEXT("UGS_GameInstance::HandleNetworkFailure - Type: %s, Error: %s"), ENetworkFailure::ToString(FailureType), *ErrorString);

    if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
    {
        MPC->HideLoadingScreen();
    }

    IOnlineSessionPtr SessionInterfacePtr = Online::GetSessionInterface(GetWorld());
    if (SessionInterfacePtr.IsValid())
    {
        // 현재 참여 중인 것으로 '착각'하고 있는 세션이 있는지 확인
        FNamedOnlineSession* CurrentSession = SessionInterfacePtr->GetNamedSession(NAME_GameSession);
        if (CurrentSession)
        {
            UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::HandleNetworkFailure - Found a lingering session. Destroying it to clean up."));

            if (!OnDestroySessionCompleteDelegateHandleForCleanup.IsValid())
            {
                OnDestroySessionCompleteDelegateHandleForCleanup = SessionInterfacePtr->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateForCleanup);
            }
            // 로컬에 남아있는 세션 파괴
            SessionInterfacePtr->DestroySession(NAME_GameSession);
        }
    }
}

void UGS_GameInstance::OnDestroySessionCompleteForCleanup(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnDestroySessionCompleteForCleanup - Session '%s' destroyed for cleanup: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

    IOnlineSessionPtr SessionInterfacePtr = Online::GetSessionInterface(GetWorld());
    if (SessionInterfacePtr.IsValid() && OnDestroySessionCompleteDelegateHandleForCleanup.IsValid())
    {
        SessionInterfacePtr->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandleForCleanup);
        OnDestroySessionCompleteDelegateHandleForCleanup.Reset();
    }
}

void UGS_GameInstance::HandlePlayerCountChanged()
{
    UWorld* World = GetWorld();
    
    if (World && (World->GetNetMode() == NM_DedicatedServer))
    {
        AGameStateBase* GS = World->GetGameState();
        if (!GS) return;

        const int32 NumPlayers = GS->PlayerArray.Num();

        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        if (Subsystem)
        {
            IOnlineSessionPtr SI = Subsystem->GetSessionInterface();
            if (SI.IsValid())
            {
                FNamedOnlineSession* CurrentSession = SI->GetNamedSession(NAME_GameSession);
                if (CurrentSession)
                {
                    if (NumPlayers == 0 && CurrentSession->SessionSettings.NumPublicConnections == 0)
                    {
                        GetWorld()->ServerTravel(DefaultLobbyMapName + "?listen", true);
                    }
                }
            }
        }
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
    HostSessionSettings->NumPublicConnections = 1; //이거 나중에 1로 바꾸기
    HostSessionSettings->NumPrivateConnections = 0;//MaxPlayers - HostSessionSettings->NumPublicConnections;
    HostSessionSettings->bShouldAdvertise = false;
    HostSessionSettings->bIsLANMatch = false;
    HostSessionSettings->bUsesPresence = true; // 스팀데디에서 이거 반드시 꺼야됨
    HostSessionSettings->bUseLobbiesIfAvailable = true;  //bUsesPresence 값이랑 동일해야함
    HostSessionSettings->bAllowJoinViaPresence = true;
    HostSessionSettings->bAllowJoinInProgress = true;
    HostSessionSettings->bAllowInvites = true;
    HostSessionSettings->bIsDedicated = IsDedicatedServerInstance();
    HostSessionSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
    HostSessionSettings->Set(SETTING_GAMEMODE, GameModePath, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
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
    UE_LOG(LogTemp, Log, TEXT("================ OnJoinSessionComplete CALLED ================"));
    UE_LOG(LogTemp, Log, TEXT("SessionName: %s"), *SessionName.ToString());

    FString ResultStr;
    switch (Result)
    {
    case EOnJoinSessionCompleteResult::Success: ResultStr = TEXT("Success"); break;
    case EOnJoinSessionCompleteResult::SessionIsFull: ResultStr = TEXT("SessionIsFull"); break;
    case EOnJoinSessionCompleteResult::SessionDoesNotExist: ResultStr = TEXT("SessionDoesNotExist"); break;
    case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress: ResultStr = TEXT("CouldNotRetrieveAddress"); break;
    case EOnJoinSessionCompleteResult::AlreadyInSession: ResultStr = TEXT("AlreadyInSession"); break;
    case EOnJoinSessionCompleteResult::UnknownError: default: ResultStr = TEXT("UnknownError"); break;
    }
    UE_LOG(LogTemp, Log, TEXT("Join Result: %s"), *ResultStr);

    APlayerController* PC = PlayerSearchingSession.Get();
    PlayerSearchingSession = nullptr;

    if (SessionInterface.IsValid() && JoinSessionCompleteDelegateHandle.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
        JoinSessionCompleteDelegateHandle.Reset();
        UE_LOG(LogTemp, Log, TEXT("OnJoinSessionComplete - Delegate handle CLEARED."));
    }

    if (Result == EOnJoinSessionCompleteResult::Success && PC)
    {
        UE_LOG(LogTemp, Log, TEXT("Join successful! Attempting to get connect string..."));
        FString ConnectString;

        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            if (bJoiningFromInvite)
            {
                ConnectString += TEXT("?bIsFromInvite=true");
                bJoiningFromInvite = false; // 플래그 사용 후 반드시 초기화
                UE_LOG(LogTemp, Log, TEXT("OnJoinSessionComplete - Appended invite flag. New ConnectString: %s"), *ConnectString);
            }
            UE_LOG(LogTemp, Log, TEXT(">>>>>>>>> Resolved ConnectString: [ %s ] <<<<<<<<<"), *ConnectString);
            PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
        }
        else
        {
            HandleNetworkFailure(GetWorld(), nullptr, ENetworkFailure::ConnectionLost, TEXT("GetResolvedConnectString failed."));
            UE_LOG(LogTemp, Error, TEXT("!!!!!!!! GetResolvedConnectString FAILED. Cannot travel. !!!!!!!!"));
        }
    }
    else
    {
        HandleNetworkFailure(GetWorld(), nullptr, ENetworkFailure::ConnectionLost, TEXT("JoinSession failed with result: ") + ResultStr);
        UE_LOG(LogTemp, Error, TEXT("!!!!!!!! JoinSession FAILED or PlayerController is invalid. Result: %s"), *ResultStr);
    }

    UE_LOG(LogTemp, Log, TEXT("================ OnJoinSessionComplete END ================"));
}

void UGS_GameInstance::GSLeaveSession(APlayerController* RequestingPlayer)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("GSLeaveSession: SessionInterface is not valid."));
        APlayerController* PC = GetFirstLocalPlayerController();
        if (PC && !MainMenuMapPath.IsEmpty())
        {
            PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
        }
        return;
    }

    FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (CurrentSession)
    {
        LeaveSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(LeaveSessionCompleteDelegate);

        if (!SessionInterface->DestroySession(NAME_GameSession))
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(LeaveSessionCompleteDelegateHandle);
            LeaveSessionCompleteDelegateHandle.Reset();

            APlayerController* PC = GetFirstLocalPlayerController();
            if (PC && !MainMenuMapPath.IsEmpty())
            {
                PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("GSLeaveSession: No active session (NAME_GameSession) found to leave. Traveling to main menu."));
        APlayerController* PC = GetFirstLocalPlayerController();
        if (PC && !MainMenuMapPath.IsEmpty())
        {
            PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
        }
    }
}

void UGS_GameInstance::OnLeaveSessionComplete(FName SessionName, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("OnLeaveSessionComplete: SessionName: %s, Success: %d"), *SessionName.ToString(), bWasSuccessful);

    // 델리게이트 핸들 정리
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(LeaveSessionCompleteDelegateHandle);
    }
    LeaveSessionCompleteDelegateHandle.Reset();

    APlayerController* PC = GetFirstLocalPlayerController();
    if (PC)
    {
        if (!MainMenuMapPath.IsEmpty())
        {
            PC->ClientTravel(MainMenuMapPath, ETravelType::TRAVEL_Absolute);
            UE_LOG(LogTemp, Log, TEXT("OnLeaveSessionComplete: Traveling to Main Menu (Client Mode): %s"), *MainMenuMapPath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("OnLeaveSessionComplete: MainMenuMapPath is empty. Cannot travel."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnLeaveSessionComplete: Failed to get PlayerController to travel to Main Menu."));
    }
}

float UGS_GameInstance::GetMouseSensitivity() const
{
    return MouseSensitivity;
}

void UGS_GameInstance::SetMouseSensitivity(float NewSensitivity)
{
    MouseSensitivity = NewSensitivity;
    SaveSettings();
}

void UGS_GameInstance::SaveSettings()
{
    if (UGS_OptionSettinsSaveGame* SaveGameInstance = Cast<UGS_OptionSettinsSaveGame>(UGameplayStatics::CreateSaveGameObject(UGS_OptionSettinsSaveGame::StaticClass())))
    {
        SaveGameInstance->MouseSensitivity = MouseSensitivity;
        UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("SettingsSlot"), 0);
    }
}

void UGS_GameInstance::LoadSettings()
{
    if (UGameplayStatics::DoesSaveGameExist(TEXT("SettingsSlot"), 0))
    {
        if (UGS_OptionSettinsSaveGame* LoadGameInstance = Cast<UGS_OptionSettinsSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SettingsSlot"), 0)))
        {
            MouseSensitivity = LoadGameInstance->MouseSensitivity;
        }
    }
}

void UGS_GameInstance::InitGameLift()
{
#if WITH_GAMELIFT
	UE_LOG(GameServerLog, Log, TEXT("Calling InitGameLift..."));

	// Getting the module first.
	FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

	//Define the server parameters for a GameLift Anywhere fleet. These are not needed for a GameLift managed EC2 fleet.
	FServerParameters ServerParametersForAnywhere;
    
    // If GameLift Anywhere is enabled, parse command line arguments and pass them in the ServerParameters object.
    SetServerParameters(ServerParametersForAnywhere);

    
    //InitSDK will establish a local connection with GameLift's agent to enable further communication.
    //Use InintSDK(ServerParametersForAnywhere) for a GameLift Anywhere fleet.
    //Use InitSDK() for a GameLift managed EC2 fleet.
    GameLiftSdkModule->InitSDK(ServerParametersForAnywhere);


    
    ProcessParameters = MakeShared<FProcessParameters>();
    
    //When a game session is created, Amazon GameLift Servers sends an activation request to the game server and passes along the game session object containing game properties and other settings.
    //Here is where a game server should take action based on the game session object.
    //Once the game server is ready to receive incoming player connections, it should invoke GameLiftServerAPI.ActivateGameSession()
    ProcessParameters->OnStartGameSession.BindLambda([=, this](Aws::GameLift::Server::Model::GameSession InGameSession)
            {
                FString GameSessionId = FString(InGameSession.GetGameSessionId());
                UE_LOG(GameServerLog, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
                
                // 1. GameLift에 세션이 활성화되었음을 먼저 알림
                GameLiftSdkModule->ActivateGameSession();

                // 2. 이제 이 정보를 바탕으로 OSS(스팀)에 세션을 등록하여 소셜 기능 활성화
                // 주의: 이 세션은 일반 서버 목록에 노출(Advertise)되지 않도록 설정하는 것이 일반적이다.
                // GameLift가 매치메이킹을 담당하므로, 스팀 서버 브라우저를 통한 접속은 막고
                // 친구 초대나 현재 플레이 중인 게임 참가 기능만 허용하기 위함이다.
                UE_LOG(LogTemp, Log, TEXT("Registering session with OnlineSubsystem (Steam) for social features."));
        
                // GSHostSession 함수 내부의 FOnlineSessionSettings에서
                // bShouldAdvertise = false; // 서버 목록에 노출 안 함
                // bAllowJoinViaPresence = true; // 친구 목록의 '게임 참가' 허용
                // 와 같이 설정하면 좋다.
                GSHostSession(DefaultMaxLobbyPlayers, NAME_GameSession, DefaultLobbyMapName, DefaultLobbyGameModePath);
            });


    
    //OnProcessTerminate callback. Amazon GameLift Servers will invoke this callback before shutting down an instance hosting this game server.
    //It gives this game server a chance to save its state, communicate with services, etc., before being shut down.
    //In this case, we simply tell Amazon GameLift Servers we are indeed going to shutdown.
    ProcessParameters->OnTerminate.BindLambda([=]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Game Server Process is terminating"));
            GameLiftSdkModule->ProcessEnding();
        });


    
    //This is the HealthCheck callback.
    //Amazon GameLift Servers will invoke this callback every 60 seconds or so.
    //Here, a game server might want to check the health of dependencies and such.
    //Simply return true if healthy, false otherwise.
    //The game server has 60 seconds to respond with its health status. Amazon GameLift Servers will default to 'false' if the game server doesn't respond in time.
    //In this case, we're always healthy!
    ProcessParameters->OnHealthCheck.BindLambda([]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Performing Health Check"));
            return true;
        });


    
    //GameServer.exe -port=7777 LOG=server.mylog
    ProcessParameters->port = FURL::UrlConfig.DefaultPort;
    TArray<FString> CommandLineTokens;
    TArray<FString> CommandLineSwitches;

    FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);

    for (const FString SwitchStr : CommandLineSwitches)
    {
        FString Key;
        FString Value;

        if (SwitchStr.Split("=", &Key, &Value))
        {
            if (Key.Equals(TEXT("port"), ESearchCase::IgnoreCase))
            {
                ProcessParameters->port = FCString::Atoi(*Value);
            }
        }
    }

    //Here, the game server tells Amazon GameLift Servers where to find game session log files.
    //At the end of a game session, Amazon GameLift Servers uploads everything in the specified 
    //location and stores it in the cloud for access later.
    TArray<FString> Logfiles;
    Logfiles.Add(TEXT("1st-Team1-Final-Project/Saved/Logs/server.log"));
    ProcessParameters->logParameters = Logfiles;

    //The game server calls ProcessReady() to tell Amazon GameLift Servers it's ready to host game sessions.
    UE_LOG(GameServerLog, Log, TEXT("Calling Process Ready..."));
    
    FGameLiftGenericOutcome ProcessReadyOutcome = GameLiftSdkModule->ProcessReady(*ProcessParameters);
    if (ProcessReadyOutcome.IsSuccess())
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(GameServerLog, Log, TEXT("Process Ready!"));
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(GameServerLog, Log, TEXT("ERROR: Process Ready Failed!"));
        FGameLiftError ProcessReadyError = ProcessReadyOutcome.GetError();
        UE_LOG(GameServerLog, Log, TEXT("ERROR: %s"), *ProcessReadyError.m_errorMessage);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(GameServerLog, Log, TEXT("InitGameLift completed!"));
    
#endif
}

void UGS_GameInstance::SetServerParameters(FServerParameters& OutServerParameters)
{
    FString glAnywhereWebSocketUrl = "";
    if (FParse::Value(FCommandLine::Get(), TEXT("-websocketurl="), glAnywhereWebSocketUrl))
    {
        OutServerParameters.m_webSocketUrl = TCHAR_TO_UTF8(*glAnywhereWebSocketUrl);
    }

    FString glAnywhereFleetId = "";
    if (FParse::Value(FCommandLine::Get(), TEXT("-fleetid="), glAnywhereFleetId))
    {
        OutServerParameters.m_fleetId = TCHAR_TO_UTF8(*glAnywhereFleetId);
    }

    FString glAnywhereHostId = "";
    if (FParse::Value(FCommandLine::Get(), TEXT("-hostid="), glAnywhereHostId))
    {
        OutServerParameters.m_hostId = TCHAR_TO_UTF8(*glAnywhereHostId);
    }

    FString glAnywhereAuthToken = "";
    if (FParse::Value(FCommandLine::Get(), TEXT("-authtoken="), glAnywhereAuthToken))
    {
        OutServerParameters.m_authToken = TCHAR_TO_UTF8(*glAnywhereAuthToken);
    }

    FString glAnywhereProcessId = "";
    if (FParse::Value(FCommandLine::Get(), TEXT("-processid="), glAnywhereProcessId))
    {
        OutServerParameters.m_processId = TCHAR_TO_UTF8(*glAnywhereProcessId);
    }
    else
    {
        // If no ProcessId is passed as a command line argument, generate a randomized unique string.
        FString TimeString = FString::FromInt(std::time(nullptr));
        FString ProcessId = "ProcessId_" + TimeString;
        OutServerParameters.m_processId = TCHAR_TO_UTF8(*ProcessId);
    }

    UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_YELLOW);
    UE_LOG(GameServerLog, Log, TEXT(">>>> WebSocket URL: %s"), *OutServerParameters.m_webSocketUrl);
    UE_LOG(GameServerLog, Log, TEXT(">>>> Fleet ID: %s"), *OutServerParameters.m_fleetId);
    UE_LOG(GameServerLog, Log, TEXT(">>>> Host ID (Compute Name): %s"), *OutServerParameters.m_hostId);
    UE_LOG(GameServerLog, Log, TEXT(">>>> Auth Token: %s"), *OutServerParameters.m_authToken);
    UE_LOG(GameServerLog, Log, TEXT(">>>> Process ID: %s"), *OutServerParameters.m_processId);
    UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
}
