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
#include "Json.h"
#include "Sound/GS_AudioManager.h"

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
    BGMVolume = 1.0f;
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
            //OnPlayerCountChanged.AddDynamic(this, &UGS_GameInstance::HandlePlayerCountChanged);
            if (OnSessionUserInviteAcceptedDelegate.IsBound()) //FOnSessionUserInviteAcceptedDelegate는 게임 인스턴스 초기화 시점부터 계속 리스닝해야 하므로 Init()에서 핸들까지 등록
            {
                OnSessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(OnSessionUserInviteAcceptedDelegate);
            }
        }
    }

    if (IsDedicatedServerInstance())
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance: Init() - This is a Dedicated Server Instance. Attempting to host a session."));

        InitGameLift();
    }

    LoadSettings();
    
    // BGM 볼륨 적용은 약간의 지연 후에 수행 (AudioManager 초기화 보장)
    FTimerHandle VolumeInitHandle;
    GetWorld()->GetTimerManager().SetTimer(VolumeInitHandle, [this]()
    {
        if (UGS_AudioManager* AudioManager = GetSubsystem<UGS_AudioManager>())
        {
            AudioManager->SetBGMVolume(BGMVolume);
        }
    }, 0.1f, false);
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
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - Leaving current session to join another."));
        DestroySessionCompleteDelegateForInviteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateForInvite);
        SessionInterface->DestroySession(NAME_GameSession);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::LeaveCurrentSessionAndJoin - No current session. Joining invite session directly."));
        
        FString GameLiftSessionId;
        if (SearchResultToJoin.Session.SessionSettings.Get(FName(TEXT("GameLiftSessionId")), GameLiftSessionId) && !GameLiftSessionId.IsEmpty())
        {
            JoinGameLiftSessionByID(GameLiftSessionId);
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
            JoinGameLiftSessionByID(GameLiftSessionId);
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
        MPC->ShowLoadingScreen();
    }

    // --- 백엔드 서비스와 통신하는 부분 ---
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

    // 2. 백엔드 서비스의 URL을 설정합니다.
    FString BackendUrl = TEXT("https://635oo4mx8l.execute-api.ap-northeast-2.amazonaws.com/stage_1/create-player-session");

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
            // 5. 백엔드로부터 받은 JSON 응답을 파싱합니다.
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (FJsonSerializer::Deserialize(Reader, JsonObject))
            {
                FString BodyString;
                if (JsonObject->TryGetStringField(TEXT("body"), BodyString))
                {
                    TSharedPtr<FJsonObject> BodyObject;
                    TSharedRef<TJsonReader<>> BodyReader = TJsonReaderFactory<>::Create(BodyString);
                    if (FJsonSerializer::Deserialize(BodyReader, BodyObject))
                    {
                        FString IpAddress = BodyObject->GetStringField(TEXT("IpAddress"));
                        FString Port = BodyObject->GetStringField(TEXT("Port"));
                        FString PlayerSessionId = BodyObject->GetStringField(TEXT("PlayerSessionId"));

                        // 6. 최종 접속 문자열을 만들어 서버로 이동합니다.
                        FString ConnectString = FString::Printf(TEXT("%s:%s?PlayerSessionId=%s"), *IpAddress, *Port, *PlayerSessionId);
                        UE_LOG(LogTemp, Log, TEXT("Successfully got connection info. Traveling to: %s"), *ConnectString);
                        
                        APlayerController* PC = GetFirstLocalPlayerController();
                        if (PC)
                        {
                             PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
                        }
                        return;
                    }
                }
            }
        }

        // 실패한 경우
        UE_LOG(LogTemp, Error, TEXT("Failed to parse connection info from backend response."));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
        {
            MPC->HideLoadingScreen();
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

void UGS_GameInstance::StartGameSessionPlacement()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem && Subsystem->GetSubsystemName() == STEAM_SUBSYSTEM)
    {
        // 1. 이전에 사용한 티켓이 있다면 명시적으로 취소하고 파기합니다.
        if (LastAuthTicketHandle != k_HAuthTicketInvalid)
        {
            SteamUser()->CancelAuthTicket(LastAuthTicketHandle);
            LastAuthTicketHandle = k_HAuthTicketInvalid;
            UE_LOG(LogTemp, Log, TEXT("Cancelled previous Steam auth ticket."));

            FPlatformProcess::Sleep(0.5f);
        }

        // 2. Steamworks API v157에 맞는 GetAuthSessionTicket 함수를 호출합니다.
        uint32 TicketSize;
        TArray<uint8> AuthTicket;
        AuthTicket.SetNum(1024); // 티켓 최대 크기
        
        // 네 번째 파라미터로 nullptr을 전달하여 로컬 유저의 티켓을 발급받습니다.
        LastAuthTicketHandle = SteamUser()->GetAuthSessionTicket(AuthTicket.GetData(), AuthTicket.Num(), &TicketSize, nullptr);

        if (LastAuthTicketHandle != k_HAuthTicketInvalid)
        {
            // 실제 티켓 크기에 맞게 배열을 조절합니다.
            AuthTicket.SetNum(TicketSize);

            // 16진수 문자열(Hex String)로 변환합니다.
            FString HexTicket;
            for (uint8 Byte : AuthTicket)
            {
                HexTicket += FString::Printf(TEXT("%02X"), Byte);
            }

            UE_LOG(LogTemp, Log, TEXT("Successfully got a new Steam ticket. Proceeding to call Lambda."));
            
            // 3. 새로 발급받은 티켓으로 람다를 호출합니다.
            FString BackendUrl = TEXT("https://635oo4mx8l.execute-api.ap-northeast-2.amazonaws.com/stage_1/start-game-session-placement");
            TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
            Request->SetURL(BackendUrl);
            Request->SetVerb("POST");
            Request->SetHeader("Content-Type", "application/json");

            FString RequestBody = FString::Printf(TEXT("{\"SteamTicket\": \"%s\"}"), *HexTicket);
            Request->SetContentAsString(RequestBody);

            Request->OnProcessRequestComplete().BindUObject(this, &UGS_GameInstance::OnStartPlacementResponse);
            Request->ProcessRequest();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get a new Steam auth ticket handle."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Steam Subsystem not found."));
    }
}

void UGS_GameInstance::OnStartPlacementResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("=== OnStartPlacementResponse Called ==="));
    UE_LOG(LogTemp, Log, TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE"));
    
    // ⭐ Request 유효성 검사
    if (!Request.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Request is INVALID (null pointer)"));
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ Request is valid"));
    UE_LOG(LogTemp, Log, TEXT("Request URL: %s"), *Request->GetURL());
    UE_LOG(LogTemp, Log, TEXT("Request Verb: %s"), *Request->GetVerb());
    
    // ⭐ Response 유효성 검사
    if (!Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Response is INVALID (null pointer)"));
        
        // Request 상태 확인
        EHttpRequestStatus::Type RequestStatus = Request->GetStatus();
        UE_LOG(LogTemp, Error, TEXT("Request Status: %d"), static_cast<int32>(RequestStatus));
        
        switch (RequestStatus)
        {
        case EHttpRequestStatus::NotStarted:
            UE_LOG(LogTemp, Error, TEXT("  → Request was not started"));
            break;
        case EHttpRequestStatus::Processing:
            UE_LOG(LogTemp, Error, TEXT("  → Request is still processing"));
            break;
        case EHttpRequestStatus::Failed:
            {
                UE_LOG(LogTemp, Error, TEXT("  → Request FAILED"));
                // 실패 원인 확인
                switch (EHttpFailureReason FailureReason = Request->GetFailureReason())
                {
                case EHttpFailureReason::ConnectionError:
                    UE_LOG(LogTemp, Error, TEXT("     Reason: Connection Error (Network issue, DNS failure, etc.)"));
                    break;
                case EHttpFailureReason::TimedOut:
                    UE_LOG(LogTemp, Error, TEXT("     Reason: Request Timed Out"));
                    break;
                case EHttpFailureReason::Other:
                    UE_LOG(LogTemp, Error, TEXT("     Reason: Other/Unknown"));
                    break;
                default:
                    UE_LOG(LogTemp, Error, TEXT("     Reason: %d"), static_cast<int32>(FailureReason));
                    break;
                }
            }
            break;
        case EHttpRequestStatus::Succeeded:
            UE_LOG(LogTemp, Error, TEXT("  → Request succeeded but response is null (Unexpected!)"));
            break;
        default:
            UE_LOG(LogTemp, Error, TEXT("  → Unknown status"));
            break;
        }
        
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ Response is valid"));

    // ⭐ bWasSuccessful이 false인 경우 상세 분석
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ bWasSuccessful is FALSE"));
        
        int32 ResponseCode = Response->GetResponseCode();
        FString ResponseBody = Response->GetContentAsString();
        
        UE_LOG(LogTemp, Error, TEXT("Response Code: %d"), ResponseCode);
        UE_LOG(LogTemp, Error, TEXT("Response Content Length: %d bytes"), ResponseBody.Len());
        
        if (ResponseCode == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("  → Response Code 0: Network timeout or connection refused"));
        }
        else if (ResponseCode >= 400 && ResponseCode < 500)
        {
            UE_LOG(LogTemp, Error, TEXT("  → Client Error (%d)"), ResponseCode);
            UE_LOG(LogTemp, Error, TEXT("  → Response Body: %s"), *ResponseBody);
        }
        else if (ResponseCode >= 500)
        {
            UE_LOG(LogTemp, Error, TEXT("  → Server Error (%d)"), ResponseCode);
            UE_LOG(LogTemp, Error, TEXT("  → Response Body: %s"), *ResponseBody);
        }
        
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
        return;
    }

    // ⭐ 여기부터는 bWasSuccessful == true
    UE_LOG(LogTemp, Log, TEXT("✅ bWasSuccessful is TRUE"));
    
    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseBody = Response->GetContentAsString();
    
    UE_LOG(LogTemp, Log, TEXT("=== Response Details ==="));
    UE_LOG(LogTemp, Log, TEXT("Response Code: %d"), ResponseCode);
    UE_LOG(LogTemp, Log, TEXT("Content Type: %s"), *Response->GetContentType());
    UE_LOG(LogTemp, Log, TEXT("Content Length: %llu bytes"), Response->GetContentLength());
    UE_LOG(LogTemp, Log, TEXT("Response Body:"));
    UE_LOG(LogTemp, Log, TEXT("%s"), *ResponseBody);
    UE_LOG(LogTemp, Log, TEXT("========================"));
    
    if (ResponseCode == 200)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
        
        if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Failed to parse JSON response."));
            UE_LOG(LogTemp, Error, TEXT("Response was: %s"), *ResponseBody);
            UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
            return;
        }
        UE_LOG(LogTemp, Log, TEXT("✅ JSON parsed successfully"));

        // JSON 구조 로깅
        UE_LOG(LogTemp, Log, TEXT("JSON Fields:"));
        for (const auto& Field : JsonObject->Values)
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s: %s"), *Field.Key, *Field.Value->AsString());
        }

        FString PlacementId;
        
        // ⭐ 시나리오 1: API Gateway가 래핑하지 않은 경우 (람다 직접 응답)
        if (JsonObject->HasField(TEXT("body")))
        {
            UE_LOG(LogTemp, Log, TEXT("✅ Detected Lambda proxy response format (has 'body' field)."));
            
            FString BodyString;
            if (JsonObject->TryGetStringField(TEXT("body"), BodyString))
            {
                UE_LOG(LogTemp, Log, TEXT("Body string: %s"), *BodyString);
                
                TSharedPtr<FJsonObject> BodyObject;
                TSharedRef<TJsonReader<>> BodyReader = TJsonReaderFactory<>::Create(BodyString);
                if (FJsonSerializer::Deserialize(BodyReader, BodyObject) && BodyObject->HasField(TEXT("PlacementId")))
                {
                    PlacementId = BodyObject->GetStringField(TEXT("PlacementId"));
                    UE_LOG(LogTemp, Log, TEXT("✅ Extracted PlacementId from body: %s"), *PlacementId);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("❌ Failed to parse body string or no PlacementId field"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ Failed to get 'body' field as string"));
            }
        }
        // ⭐ 시나리오 2: API Gateway가 언래핑한 경우 (바로 PlacementId)
        else if (JsonObject->HasField(TEXT("PlacementId")))
        {
            UE_LOG(LogTemp, Log, TEXT("✅ Detected unwrapped response format (direct PlacementId)."));
            PlacementId = JsonObject->GetStringField(TEXT("PlacementId"));
            UE_LOG(LogTemp, Log, TEXT("✅ Extracted PlacementId: %s"), *PlacementId);
        }
        // ⭐ 시나리오 3: 에러 응답
        else if (JsonObject->HasField(TEXT("error")))
        {
            FString ErrorMsg = JsonObject->GetStringField(TEXT("error"));
            UE_LOG(LogTemp, Error, TEXT("❌ Lambda returned error: %s"), *ErrorMsg);
            UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
            return;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Unknown response format. No 'body', 'PlacementId', or 'error' field found."));
            UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
            return;
        }

        if (!PlacementId.IsEmpty())
        {
            CurrentPlacementId = PlacementId;
            UE_LOG(LogTemp, Log, TEXT("✅✅✅ SUCCESS! Got PlacementId: %s. Starting to poll..."), *CurrentPlacementId);

            // 다음 단계: 3초마다 상태 확인 타이머 시작
            GetWorld()->GetTimerManager().SetTimer(PollPlacementTimerHandle, this, &UGS_GameInstance::PollPlacementStatus, 3.0f, true);
            return;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ PlacementId is empty after parsing."));
            UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
        }
    }
    else if (ResponseCode == 401)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Steam authentication failed (401 Unauthorized)."));
        UE_LOG(LogTemp, Error, TEXT("Response Body: %s"), *ResponseBody);
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
    }
    else if (ResponseCode >= 400 && ResponseCode < 500)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Client error: %d"), ResponseCode);
        UE_LOG(LogTemp, Error, TEXT("Response Body: %s"), *ResponseBody);
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
    }
    else if (ResponseCode >= 500)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Server error: %d"), ResponseCode);
        UE_LOG(LogTemp, Error, TEXT("Response Body: %s"), *ResponseBody);
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Unexpected response code: %d"), ResponseCode);
        UE_LOG(LogTemp, Error, TEXT("Response Body: %s"), *ResponseBody);
        UE_LOG(LogTemp, Error, TEXT("Failed to start game session placement."));
    }
}

void UGS_GameInstance::PollPlacementStatus()
{
    FString BackendUrl = TEXT("https://635oo4mx8l.execute-api.ap-northeast-2.amazonaws.com/stage_1/start-game-session-placement");

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendUrl);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    FString RequestBody = FString::Printf(TEXT("{\"PlacementId\": \"%s\"}"), *CurrentPlacementId);
    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindUObject(this, &UGS_GameInstance::OnPollPlacementResponse);
    Request->ProcessRequest();
}

void UGS_GameInstance::OnPollPlacementResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        // 1. 람다의 전체 응답을 파싱합니다.
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            // 2. "body" 필드를 문자열로 가져옵니다.
            FString BodyString;
            if (JsonObject->TryGetStringField(TEXT("body"), BodyString))
            {
                // 3. 그 "body" 문자열을 다시 파싱하여 상태 정보를 얻습니다.
                TSharedPtr<FJsonObject> BodyObject;
                TSharedRef<TJsonReader<>> BodyReader = TJsonReaderFactory<>::Create(BodyString);
                if (FJsonSerializer::Deserialize(BodyReader, BodyObject) && BodyObject->HasField(TEXT("Status")))
                {
                    FString Status = BodyObject->GetStringField(TEXT("Status"));
                    UE_LOG(LogTemp, Log, TEXT("Polling... GameSession status is: %s"), *Status);

                    if (Status == TEXT("FULFILLED"))
                    {
                        // 최종 목표 달성! 타이머를 중지합니다.
                        GetWorld()->GetTimerManager().ClearTimer(PollPlacementTimerHandle);

                        FString IpAddress, Port, PlayerSessionId;

                        // TryGetStringField를 사용하여 모든 필드가 유효한 문자열인지 안전하게 확인합니다.
                        if (BodyObject->TryGetStringField(TEXT("IpAddress"), IpAddress) &&
                            BodyObject->TryGetStringField(TEXT("Port"), Port) &&
                            BodyObject->TryGetStringField(TEXT("PlayerSessionId"), PlayerSessionId) &&
                            !IpAddress.IsEmpty() && !Port.IsEmpty() && !PlayerSessionId.IsEmpty())
                        {
                            // 모든 정보가 유효할 때만 접속을 시도합니다.
                            FString ConnectString = FString::Printf(TEXT("%s:%s?PlayerSessionId=%s"), *IpAddress, *Port, *PlayerSessionId);

                            UE_LOG(LogTemp, Log, TEXT("Placement FULFILLED! Traveling to: %s"), *ConnectString);
                            
                            APlayerController* PC = GetFirstLocalPlayerController();
                            if (PC)
                            {
                                PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
                            }
                        }
                        else
                        {
                            // FULFILLED 상태이지만, 필수 정보 중 하나가 누락/null인 경우
                            UE_LOG(LogTemp, Error, TEXT("Placement FULFILLED, but connection info is invalid or missing."));
                            if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
                            {
                                MPC->HideLoadingScreen();
                            }
                        }
                    }
                    else if (Status == TEXT("TIMED_OUT") || Status == TEXT("FAILED") || Status == TEXT("CANCELLED"))
                    {
                        // 실패 상태 처리
                        GetWorld()->GetTimerManager().ClearTimer(PollPlacementTimerHandle);
                        UE_LOG(LogTemp, Error, TEXT("Placement failed with status: %s"), *Status);
                        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
                        {
                            MPC->HideLoadingScreen();
                        }
                    }
                    // "PENDING" 상태일 경우, 아무것도 하지 않고 타이머가 다음 주기에 다시 호출하기를 기다립니다.
                    return; // 성공적으로 상태를 확인했으므로 함수 종료
                }
            }
        }
    }
    
    // HTTP 요청 자체가 실패했거나, JSON 파싱에 실패한 경우
    UE_LOG(LogTemp, Warning, TEXT("Failed to poll game session status. Retrying in 3 seconds..."));
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

void UGS_GameInstance::CheckIfLastPlayerAndTerminate()
{
    UWorld* World = GetWorld();
    if (World && (World->GetNetMode() == ENetMode::NM_DedicatedServer || World->GetNetMode() == ENetMode::NM_ListenServer))
    {
        AGameStateBase* GS = World->GetGameState();
        
        // Logout 함수는 플레이어 컨트롤러가 GameState의 PlayerArray에서 실제로 제거되기 전에 호출
        // 플레이어 수가 1명이라면, 마지막 플레이어가 로그아웃하는 과정임을 의미
        if (GS && GS->PlayerArray.Num() == 1)
        {
            UE_LOG(LogTemp, Log, TEXT("Last player has left the game session. Calling ProcessEnding()."));
            
#if WITH_GAMELIFT_AUTOMATION
            FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));
            if (GameLiftSdkModule)
            {
                GameLiftSdkModule->ProcessEnding();
            }
#endif
        }
        else if (GS)
        {
            UE_LOG(LogTemp, Log, TEXT("A player has left. Players remaining: %d"), GS->PlayerArray.Num() - 1);
        }
    }
}

void UGS_GameInstance::StorePlayerSession(const FUniqueNetIdRepl& PlayerId, const FString& PlayerSessionId)
{
    if (PlayerId.IsValid() && !PlayerSessionId.IsEmpty())
    {
        // FUniqueNetId를 문자열로 변환하여 Key로 사용합니다.
        PlayerSessionLinks.Add(PlayerId.ToString(), PlayerSessionId);
        UE_LOG(LogTemp, Log, TEXT("GameInstance: Stored PlayerSession '%s' for Player '%s'"), *PlayerSessionId, *PlayerId.ToString());
    }
}

FString UGS_GameInstance::RemoveAndGetPlayerSession(const FUniqueNetIdRepl& PlayerId)
{
    if (PlayerId.IsValid())
    {
        const FString PlayerIdString = PlayerId.ToString();
        
        FString* FoundSessionIdPtr = PlayerSessionLinks.Find(PlayerIdString);

        if (FoundSessionIdPtr != nullptr)
        {
            const FString FoundSessionId = *FoundSessionIdPtr;
            PlayerSessionLinks.Remove(PlayerIdString);
            return FoundSessionId;
        }
    }
    return FString();
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
        SaveGameInstance->BGMVolume = BGMVolume;

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
            BGMVolume = LoadGameInstance->BGMVolume;
        }
    }
    else
    {
        // 저장 파일이 없는 경우 기본값 설정
        MouseSensitivity = 1.0f;
        BGMVolume = 1.0f;
    }
}

float UGS_GameInstance::GetBGMVolume() const
{
    return BGMVolume;
}

void UGS_GameInstance::SetBGMVolume(float NewVolume)
{
    const float ClampedVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

    // 값이 변경되지 않았으면 저장하지 않음 (성능 최적화)
    if (FMath::IsNearlyEqual(BGMVolume, ClampedVolume, 0.001f))
    {
        return;
    }

    BGMVolume = ClampedVolume;

    // AudioManager를 통해 실제 볼륨 적용
    if (UGS_AudioManager* AudioManager = GetSubsystem<UGS_AudioManager>())
    {
        AudioManager->SetBGMVolume(BGMVolume);
    }

    SaveSettings();
}

void UGS_GameInstance::InitGameLift()
{
#if WITH_GAMELIFT
    UE_LOG(GameServerLog, Log, TEXT("Calling InitGameLift..."));

    // Getting the module first.
    FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));
    
    // --- ProcessParameters 객체를 먼저 생성하고 공통 설정을 채웁니다 ---
    ProcessParameters = MakeShared<FProcessParameters>();
    
    // OnStartGameSession 콜백 설정
    ProcessParameters->OnStartGameSession.BindLambda([=, this](Aws::GameLift::Server::Model::GameSession InGameSession)
    {
        FString GameSessionId = FString(InGameSession.GetGameSessionId());
        UE_LOG(GameServerLog, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
        
        GameLiftSdkModule->ActivateGameSession();

        int32 AssignedPort = InGameSession.GetPort();
        UE_LOG(GameServerLog, Log, TEXT("GameSession assigned to port: %d"), AssignedPort);

        AsyncTask(ENamedThreads::GameThread, [=, this]()
        {
            UWorld* World = GetWorld();
            if (World)
            {
                FString MapLoadOptions = FString::Printf(TEXT("?listen?Port=%d"), AssignedPort);
            FString MapToLoad = FString(*DefaultLobbyMapName) + MapLoadOptions;
            
            UGameplayStatics::OpenLevel(World, FName(*MapToLoad), true);
            UE_LOG(GameServerLog, Log, TEXT("Server traveling to map: %s"), *MapToLoad);
            }
        });
    });

    // OnTerminate 콜백 설정
    ProcessParameters->OnTerminate.BindLambda([=]()
    {
        UE_LOG(GameServerLog, Log, TEXT("Game Server Process is terminating"));
        GameLiftSdkModule->ProcessEnding();
    });

    // OnHealthCheck 콜백 설정
    ProcessParameters->OnHealthCheck.BindLambda([]()
    {
        UE_LOG(GameServerLog, Log, TEXT("Performing Health Check"));
        return true;
    });

    // 로그 파일 경로 지정 (두 버전 모두에 적용)
    TArray<FString> Logfiles;
#if WITH_GAMELIFT_AUTOMATION
    Logfiles.Add(TEXT("C:/game/logs/server.log")); // EC2 인스턴스 내의 경로
#else
    Logfiles.Add(TEXT("1st-Team1-Final-Project/Saved/Logs/server.log")); // Anywhere (로컬) 경로
#endif
    ProcessParameters->logParameters = Logfiles;

    ProcessParameters->port = FURL::UrlConfig.DefaultPort;

#if WITH_GAMELIFT_AUTOMATION
    // --- EC2 버전일 경우 ---
    UE_LOG(GameServerLog, Log, TEXT("EC2 mode enabled. Initializing SDK for EC2 Fleet."));
    GameLiftSdkModule->InitSDK();
#else
    // --- Anywhere / LAN 버전일 경우 ---
    UE_LOG(GameServerLog, Log, TEXT("EC2 mode disabled. Initializing SDK for Anywhere Fleet / LAN."));
    
    FServerParameters ServerParametersForAnywhere;
    SetServerParameters(ServerParametersForAnywhere);
    GameLiftSdkModule->InitSDK(ServerParametersForAnywhere);
#endif
    
    // --- ProcessReady()는 모든 설정이 끝난 후 마지막에 한 번만 호출합니다 ---
    UE_LOG(GameServerLog, Log, TEXT("Calling Process Ready..."));
    FGameLiftGenericOutcome ProcessReadyOutcome = GameLiftSdkModule->ProcessReady(*ProcessParameters);
    
    if (ProcessReadyOutcome.IsSuccess())
    {
        UE_LOG(GameServerLog, Log, TEXT("Process Ready SUCCESS!"));
    }
    else
    {
        UE_LOG(GameServerLog, Error, TEXT("Process Ready FAILED!"));
        FGameLiftError ProcessReadyError = ProcessReadyOutcome.GetError();
        UE_LOG(GameServerLog, Error, TEXT("Error: %s"), *ProcessReadyError.m_errorMessage);
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

