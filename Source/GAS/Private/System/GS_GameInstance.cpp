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
#include "Async/Async.h"

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

        InitGameLift();
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
        return;
    }

    FNamedOnlineSession* CurrentSession = SessionInterface->GetNamedSession(NAME_GameSession);
    if (CurrentSession != nullptr && CurrentSession->SessionState != EOnlineSessionState::NoSession)
    {
        // 현재 세션이 있으므로, 파괴를 요청하고 콜백(OnDestroySessionCompleteForInvite)을 기다립니다.
        // 이 부분은 이미 코드가 올바르게 작성되어 있을 것입니다.
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - Leaving current session to join another."));
        DestroySessionCompleteDelegateForInviteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInvite);
        SessionInterface->DestroySession(NAME_GameSession);
    }
    else
    {
        // 현재 참여 중인 세션이 없으므로, 바로 초대받은 세션으로 참여를 시도합니다.
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - No current session. Joining invite session directly."));
        
        FString GameLiftSessionId;
        if (SearchResultToJoin.Session.SessionSettings.Get(FName(TEXT("GameLiftSessionId")), GameLiftSessionId) && !GameLiftSessionId.IsEmpty())
        {
            JoinGameLiftSessionByID(GameLiftSessionId); // 새로 만든 GameLift 접속 함수 호출
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - Could not find GameLiftSessionId in invite."));
        }
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

    if (InviteSessionToJoinAfterDestroy.IsValid())
    {
        FString GameLiftSessionId;
        if (InviteSessionToJoinAfterDestroy.Session.SessionSettings.Get(FName(TEXT("GameLiftSessionId")), GameLiftSessionId) && !GameLiftSessionId.IsEmpty())
        {
            JoinGameLiftSessionByID(GameLiftSessionId); // 새로 만든 GameLift 접속 함수 호출
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnDestroySessionCompleteForInvite - Could not find GameLiftSessionId in invite after destroying session."));
        }
    }
    PlayerJoiningFromInvite = nullptr;
}

void UGS_GameInstance::OnSessionUserInviteAccepted_Impl(const bool bWasSuccessful, const int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
    UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnSessionUserInviteAccepted_Impl - Invite accepted by LocalUserNum: %d, Success: %s"), ControllerId, bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (bWasSuccessful && InviteResult.IsValid())
    {
        // 1. InviteResult에서 GameLift 세션 ID를 추출합니다.
        //    이것은 초대자가 초대장에 세션 ID를 어떻게 심었는지에 따라 달라집니다.
        //    일반적으로 ConnectString에 저장됩니다.
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(InviteResult, NAME_Default, ConnectString))
        {
            // 예시: ConnectString이 "GameLiftSessionId=gsess-xxxx" 형태라고 가정
            FString GameLiftSessionId = UGameplayStatics::ParseOption(ConnectString, TEXT("GameLiftSessionId"));

            if (!GameLiftSessionId.IsEmpty())
            {
                UE_LOG(LogTemp, Log, TEXT("Extracted GameLift Session ID from invite: %s"), *GameLiftSessionId);

                // 2. 추출한 ID로 GameLift 세션에 참여하는 새로운 함수 호출
                JoinGameLiftSessionByID(GameLiftSessionId);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Could not find GameLiftSessionId in invite connect string."));
            }
        }
    }
}

void UGS_GameInstance::JoinGameLiftSessionByID(const FString& GameLiftSessionId)
{
    UE_LOG(LogTemp, Log, TEXT("Attempting to join GameLift session by ID: %s"), *GameLiftSessionId);
    if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
    {
        MPC->ShowLoadingScreen(); // 로딩 UI 표시
    }

    // --- 백엔드 서비스와 통신하는 부분 ---
    // 실제로는 AWS Lambda와 API Gateway를 사용하여 이 부분을 구현해야 합니다.

    // 1. 플레이어의 Steam 인증 티켓을 가져옵니다.
    FString SteamAuthTicket;
    IOnlineIdentityPtr Identity = IOnlineSubsystem::Get()->GetIdentityInterface();
    if (Identity.IsValid())
    {
        SteamAuthTicket = Identity->GetAuthToken(0);
    }

    if (SteamAuthTicket.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get Steam Auth Ticket. Cannot join session."));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
        {
            MPC->HideLoadingScreen();
        }
        return;
    }

    // 2. 백엔드 서비스의 URL을 설정합니다. (직접 구축한 API Gateway의 엔드포인트로 변경해야 합니다)
    FString BackendUrl = TEXT("https://your-api-gateway-endpoint.com/join"); // <--- 이 URL을 실제 URL로 변경하세요.

    // 3. HTTP 요청을 생성합니다.
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendUrl);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    // Body에 GameLiftSessionId와 플레이어의 Steam 인증 티켓을 담습니다.
    FString RequestBody = FString::Printf(TEXT("{\"SessionId\": \"%s\", \"SteamTicket\": \"%s\"}"), *GameLiftSessionId, *SteamAuthTicket);
    Request->SetContentAsString(RequestBody);

    // 4. 요청 완료 콜백을 바인딩합니다.
    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
        {
            // 1. 백엔드로부터 받은 JSON 응답을 파싱합니다.
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                FString IpAddress = JsonObject->GetStringField("IpAddress");
                FString Port = JsonObject->GetStringField("Port");
                FString PlayerSessionId = JsonObject->GetStringField("PlayerSessionId");

                // 2. 최종 접속 문자열을 만들어 서버로 이동합니다.
                FString ConnectString = FString::Printf(TEXT("%s:%s?PlayerSessionId=%s"), *IpAddress, *Port, *PlayerSessionId);
                
                UE_LOG(LogTemp, Log, TEXT("Successfully got connection info. Traveling to: %s"), *ConnectString);
                
                APlayerController* PC = GetFirstLocalPlayerController();
                if (PC)
                {
                     PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get connection info from backend."));
            if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
            {
                MPC->HideLoadingScreen();
            }
        }
    });

    // 7. HTTP 요청을 보냅니다.
    Request->ProcessRequest();
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

void UGS_GameInstance::GSLeaveSession(APlayerController* RequestingPlayer) //NetDriver 상관 없이 필요함
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

                // 2. 스팀 세션 생성(GSHostSession) 대신, 서버가 직접 로비 레벨로 이동
                AsyncTask(ENamedThreads::GameThread, [=, this]()
                {
                    UWorld* World = GetWorld();
                    if (World)
                    {
                        FURL TravelURL;
                        TravelURL.Map = DefaultLobbyMapName;
                        TravelURL.AddOption(TEXT("listen"));
                        World->ServerTravel(TravelURL.ToString(), true);
                        UE_LOG(GameServerLog, Log, TEXT("Server traveling to map: %s"), *DefaultLobbyMapName);
                    }
                });
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
