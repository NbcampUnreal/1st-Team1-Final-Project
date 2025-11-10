#include "System/GS_GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h" //SETTING_GAMEMODE 이런 거 쓸라면 필요. 앞으로 까먹지 말기
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "System/Save/GS_OptionSettinsSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "System/PlayerController/GS_MainMenuPC.h"
#include "Async/Async.h"
#include "Json.h"
#include "GameFramework/PlayerState.h"
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
    SFXVolume = 1.0f;
    LanguageSet = "ko";
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
        FString GameLiftSessionId;

        // 초대장의 세션 설정에서 직접 GameLiftSessionId를 읽어옵니다.
        if (InviteResult.Session.SessionSettings.Get(FName(TEXT("GameLiftSessionId")), GameLiftSessionId) && !GameLiftSessionId.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("Extracted GameLift Session ID from invite session settings: %s"), *GameLiftSessionId);
            JoinGameLiftSessionByID(GameLiftSessionId);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not find GameLiftSessionId in invite session settings."));
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
    APlayerController* PC = GetFirstLocalPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("JoinGameLiftSessionByID: Failed to get FirstLocalPlayerController."));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }

    APlayerState* PS = PC->PlayerState;
    if (!PS || !PS->GetUniqueId().IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("JoinGameLiftSessionByID: Failed to get a valid PlayerState or UniqueId."));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }
    FString PlayerId = PS->GetUniqueId().ToString();

    FString BackendUrl = TEXT("https://635oo4mx8l.execute-api.ap-northeast-2.amazonaws.com/stage_1/create-player-session");

    // HTTP 요청을 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendUrl);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    // Body 구성
    FString RequestBody = FString::Printf(TEXT("{\"SessionId\": \"%s\", \"PlayerId\": \"%s\"}"), *GameLiftSessionId, *PlayerId);
    Request->SetContentAsString(RequestBody);

    // 요청 완료 콜백 바인딩
    Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bWasSuccessful)
    {
        if (!(bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200))
        {
            UE_LOG(LogTemp, Error, TEXT("JoinGL: HTTP fail. ok=%d code=%d"), bWasSuccessful?1:0, Response.IsValid()?Response->GetResponseCode():-1);
            if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
            return;
        }

        const FString Raw = Response->GetContentAsString();

        // 1) 1차 역직렬화
        TSharedPtr<FJsonObject> Root;
        {
            TSharedRef<TJsonReader<>> R = TJsonReaderFactory<>::Create(Raw);
            if (!FJsonSerializer::Deserialize(R, Root) || !Root.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("JoinGL: JSON root parse failed. raw='%s'"), *Raw);
                if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
                return;
            }
        }

        // 2) 래핑/언래핑 모두 처리
        TSharedPtr<FJsonObject> Body = Root;
        FString BodyStr;
        if (Root->TryGetStringField(TEXT("body"), BodyStr))
        {
            UE_LOG(LogTemp, Log, TEXT("JoinGL: Detected Lambda proxy response (has 'body')."));
            TSharedRef<TJsonReader<>> BR = TJsonReaderFactory<>::Create(BodyStr);
            if (!FJsonSerializer::Deserialize(BR, Body) || !Body.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("JoinGL: inner 'body' JSON parse failed. body='%s'"), *BodyStr);
                if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("JoinGL: Detected unwrapped response."));
        }

        // 3) 키 대소문자/타입(문자/숫자) 호환 도우미
        auto ReadStr = [](const TSharedPtr<FJsonObject>& Obj, const TCHAR* Upper, const TCHAR* Lower, FString& Out)->bool
        {
            return Obj->TryGetStringField(Upper, Out) || Obj->TryGetStringField(Lower, Out);
        };
        auto ReadStrOrNum = [](const TSharedPtr<FJsonObject>& Obj, const TCHAR* Upper, const TCHAR* Lower, FString& Out)->bool
        {
            if (Obj->TryGetStringField(Upper, Out) || Obj->TryGetStringField(Lower, Out)) return !Out.IsEmpty();
            double Num = 0.0;
            if (Obj->TryGetNumberField(Upper, Num) || Obj->TryGetNumberField(Lower, Num)) { Out = FString::Printf(TEXT("%.0f"), Num); return true; }
            return false;
        };

        // 4) 실제 필드 읽기 (대소문자 혼용 허용)
        FString IpAddress, Port, PlayerSessionId;
        const bool OkIp   = ReadStr(Body, TEXT("IpAddress"),      TEXT("ipAddress"),      IpAddress) || ReadStr(Body, TEXT("Ip"), TEXT("ip"), IpAddress);
        const bool OkPort = ReadStrOrNum(Body, TEXT("Port"),      TEXT("port"),           Port);
        const bool OkPs   = ReadStr(Body, TEXT("PlayerSessionId"),TEXT("playerSessionId"),PlayerSessionId) ||
                            ReadStr(Body, TEXT("PlayerSessionID"),TEXT("playerSessionID"),PlayerSessionId);

        if (!OkIp || !OkPort || !OkPs || PlayerSessionId.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("JoinGL: missing fields. Ip[%d]=%s Port[%d]=%s PlayerSessionId[%d]=%s"),
                OkIp?1:0, *IpAddress, OkPort?1:0, *Port, OkPs?1:0, *PlayerSessionId);
            UE_LOG(LogTemp, Error, TEXT("JoinGL: raw='%s'"), *Raw);
            if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
            return;
        }

        // 5) 접속
        const FString Connect = FString::Printf(TEXT("%s:%s?PlayerSessionId=%s"), *IpAddress, *Port, *PlayerSessionId);
        UE_LOG(LogTemp, Log, TEXT("JoinGL: Traveling to %s"), *Connect);

        if (APlayerController* PC = GetFirstLocalPlayerController())
        {
            PC->ClientTravel(Connect, ETravelType::TRAVEL_Absolute);
        }
    });

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

void UGS_GameInstance::OnStart()
{
    Super::OnStart();

    // 저장된 옵션 세팅 로드
    LoadSettings();

    // AudioManager에 로드된 볼륨 값 적용
    if (UGS_AudioManager* AudioManager = GetSubsystem<UGS_AudioManager>())
    {
        AudioManager->SetBGMVolume(BGMVolume);
        AudioManager->SetSFXVolume(SFXVolume);
        //UE_LOG(LogTemp, Log, TEXT("UGS_GameInstance::OnStart - Applied saved volumes: BGM=%.2f, SFX=%.2f"), BGMVolume, SFXVolume);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UGS_GameInstance::OnStart - AudioManager not available yet"));
    }

    FTextLocalizationManager::Get().RefreshResources();
}

void UGS_GameInstance::StartGameSession()
{
    UE_LOG(LogTemp, Log, TEXT("Requesting new game session..."));

    APlayerController* PC = GetFirstLocalPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("StartGameSession: Failed to get FirstLocalPlayerController."));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }

    APlayerState* PS = PC->PlayerState;
    if (!PS || !PS->GetUniqueId().IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("StartGameSession: Failed to get a valid PlayerState or UniqueId."));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }

    FString HostPlayerId = PS->GetUniqueId().ToString(); // 지금은 이거 aws 람다로 굳이 넘길 필요 없는데 나중에 스팀 쪽에서도 검증 강화해야 할 것 같음 그때 필요함
    
    UE_LOG(LogTemp, Log, TEXT("StartGameSession: Host PlayerId is: %s"), *HostPlayerId);

    
    FString BackendUrl = TEXT("https://635oo4mx8l.execute-api.ap-northeast-2.amazonaws.com/stage_1/create-game-session");
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendUrl);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");

    FString RequestBody = FString::Printf(TEXT("{\"AliasId\": \"%s\", \"PlayerId\": \"%s\"}"), *TargetAliasId, *HostPlayerId);
    Request->SetContentAsString(RequestBody);
    Request->OnProcessRequestComplete().BindUObject(this, &UGS_GameInstance::OnCreateSessionResponse);
    Request->ProcessRequest();
}

void UGS_GameInstance::OnCreateSessionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Log, TEXT("=== OnCreateSessionResponse Called ==="));
    UE_LOG(LogTemp, Log, TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE"));
    
    // Request 유효성 검사
    if (!Request.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Request is INVALID (null pointer)"));
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }

    // Response 유효성 검사
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
        
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }

    // bWasSuccessful이 false인 경우 상세 분석
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ bWasSuccessful is FALSE"));
        
        int32 ResponseCode = Response->GetResponseCode();
        FString ResponseBody = Response->GetContentAsString();
        
        UE_LOG(LogTemp, Error, TEXT("Response Code: %d"), ResponseCode);
        UE_LOG(LogTemp, Error, TEXT("  → Response Body: %s"), *ResponseBody);
        
        if (ResponseCode == 0)
        {
            UE_LOG(LogTemp, Error, TEXT("  → Response Code 0: Network timeout or connection refused"));
        }
        else if (ResponseCode >= 400 && ResponseCode < 500)
        {
            UE_LOG(LogTemp, Error, TEXT("  → Client Error (%d)"), ResponseCode);
        }
        else if (ResponseCode >= 500)
        {
            UE_LOG(LogTemp, Error, TEXT("  → Server Error (%d)"), ResponseCode);
        }
        
        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
        return;
    }

    // [로직 시작]
    // 여기까지 왔다면 bWasSuccessful == true 이고, Request/Response가 유효함.
    
    int32 ResponseCode = Response->GetResponseCode();
    FString ResponseBody = Response->GetContentAsString();
    
    UE_LOG(LogTemp, Log, TEXT("=== Response Details ==="));
    UE_LOG(LogTemp, Log, TEXT("Response Code: %d"), ResponseCode);
    UE_LOG(LogTemp, Log, TEXT("Content Type: %s"), *Response->GetContentType());
    UE_LOG(LogTemp, Log, TEXT("Response Body: %s"), *ResponseBody);
    UE_LOG(LogTemp, Log, TEXT("========================"));
    
    // ResponseCode == 200 인지 확인하고, 파싱하여 작업 수행
    if (ResponseCode == 200)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("✅ Main JSON parsed successfully"));
            
            FString BodyString;
            TSharedPtr<FJsonObject> BodyObject; // 최종 데이터가 담길 JSON 객체

            if (JsonObject->HasField(TEXT("body")))
            {
                // API Gateway 프록시 응답: "body" 필드 안의 문자열을 다시 파싱
                UE_LOG(LogTemp, Log, TEXT("Detected Lambda proxy response (has 'body' field)."));
                BodyString = JsonObject->GetStringField(TEXT("body"));
                TSharedRef<TJsonReader<>> BodyReader = TJsonReaderFactory<>::Create(BodyString);
                
                if (!FJsonSerializer::Deserialize(BodyReader, BodyObject) || !BodyObject.IsValid())
                {
                    UE_LOG(LogTemp, Error, TEXT("❌ Failed to parse inner 'body' JSON string."));
                    // (실패 처리 - 로딩 화면 숨기기 등)
                }
            }
            else
            {
                // API Gateway가 래핑하지 않은 응답
                UE_LOG(LogTemp, Log, TEXT("Detected unwrapped response."));
                BodyObject = JsonObject; // 루트 객체가 이미 최종 데이터임
            }

            // BodyObject가 유효한지 확인 (람다가 에러를 반환했을 수 있으므로)
            if (BodyObject.IsValid() && BodyObject->HasField(TEXT("Status")))
            {
                FString Status = BodyObject->GetStringField(TEXT("Status"));
                UE_LOG(LogTemp, Log, TEXT("GameSession status is: %s"), *Status);

                if (Status == TEXT("FULFILLED"))
                {
                    FString IpAddress, Port, PlayerSessionId;

                    if (BodyObject->TryGetStringField(TEXT("IpAddress"), IpAddress) &&
                        BodyObject->TryGetStringField(TEXT("Port"), Port) &&
                        BodyObject->TryGetStringField(TEXT("PlayerSessionId"), PlayerSessionId)) 
                    {
                        if (PlayerSessionId.IsEmpty())
                        {
                            UE_LOG(LogTemp, Error, TEXT("PlayerSessionId parsing succeeded, but the string is EMPTY! Lambda might have sent null."));
                            if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
                            return; // 함수 종료
                        }

                        FString ConnectString = FString::Printf(TEXT("%s:%s?PlayerSessionId=%s"), *IpAddress, *Port, *PlayerSessionId);
                        UE_LOG(LogTemp, Log, TEXT("Attempting ClientTravel with ConnectString: %s"), *ConnectString);
        
                        APlayerController* PC = GetFirstLocalPlayerController();
                        if (PC)
                        {
                            PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Session creation FULFILLED, but connection info is invalid or missing."));
                        if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
                    }
                }
                else // Status == "FAILED" 등
                {
                    UE_LOG(LogTemp, Error, TEXT("Session creation failed with status: %s"), *Status);
                    if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController())) { MPC->HideLoadingScreen(); }
                }
                
                return; // 성공/실패 처리 완료 후 함수 종료
            }
            else
            {
                 // 'Status' 필드가 없는 경우 (람다가 에러를 반환한 경우)
                 UE_LOG(LogTemp, Error, TEXT("❌ Parsed 'body' object, but 'Status' field is missing. Lambda returned error."));
                 if (BodyObject.IsValid())
                 {
                    FString ErrorMsg;
                    if(BodyObject->TryGetStringField(TEXT("error"), ErrorMsg))
                    {
                        UE_LOG(LogTemp, Error, TEXT("Lambda Error: %s"), *ErrorMsg);
                    }
                 }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Failed to parse main JSON response. Body: %s"), *ResponseBody);
        }
    }
    else
    {
        // bWasSuccessful=true 였는데도 200이 아닌 경우 (이론상 2xx, 3xx 등)
        UE_LOG(LogTemp, Error, TEXT("❌ HTTP Request was successful but ResponseCode is not 200 (%d). Body: %s"), ResponseCode, *ResponseBody);
    }
    
    // 여기까지 왔다면 HTTP 에러거나 JSON 파싱 에러임
    UE_LOG(LogTemp, Error, TEXT("Failed to create game session (HTTP Error or Parse Error)."));
    if (AGS_MainMenuPC* MPC = Cast<AGS_MainMenuPC>(GetFirstLocalPlayerController()))
    {
        MPC->HideLoadingScreen();
    }
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
        
        if (GS && GS->PlayerArray.Num() == 1) // 마지막 플레이어가 로그아웃 중
        {
            UE_LOG(LogTemp, Log, TEXT("마지막 플레이어가 나갔습니다. TerminateServerProcess 호출."));
            TerminateServerProcess();
        }
        else if (GS)
        {
            UE_LOG(LogTemp, Log, TEXT("플레이어가 나갔습니다. 남은 인원: %d"), GS->PlayerArray.Num() - 1);
        }
    }
}

void UGS_GameInstance::TerminateServerProcess()
{
#if WITH_GAMELIFT
    FGameLiftServerSDKModule* GameLiftSdkModule = FModuleManager::GetModulePtr<FGameLiftServerSDKModule>(TEXT("GameLiftServerSDK"));
    if (GameLiftSdkModule)
    {
        GameLiftSdkModule->ProcessEnding();

        //2초 후 강제 종료 실행
        FTimerHandle ShutdownTimer;
        GetWorld()->GetTimerManager().SetTimer(ShutdownTimer, [this]()
        {
            UE_LOG(LogTemp, Log, TEXT("서버 프로세스 종료를 시작합니다 (0명 로그아웃 또는 시작 타임아웃)."));
            FGenericPlatformMisc::RequestExit(true);
        }, 2.0f, false);
    }
#endif
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

#if WITH_GAMELIFT
void UGS_GameInstance::StartHealthAndIdleMonitor()
{
    if (!bGameSessionActive) return;
    if (UWorld* World = GetWorld())
    {
        if (!World->GetTimerManager().IsTimerActive(HealthIdleTimer))
        {
            World->GetTimerManager().SetTimer(
                HealthIdleTimer,
                this, &UGS_GameInstance::TickHealthAndIdle,
                HealthTickSeconds,
                true,
                HealthTickSeconds
            );
            UE_LOG(GameServerLog, Log, TEXT("Health/Idle monitor started: tick=%.1fs, idleGrace=%.1fs"),
                   HealthTickSeconds, IdleShutdownGraceSeconds);
        }
    }
}

void UGS_GameInstance::TickHealthAndIdle()
{
    if (!bGameSessionActive) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const double Now = World->GetRealTimeSeconds();

    // 현재 GameMode가 무엇이든, GameState의 PlayerArray로 현재 접속 인원 파악
    AGameStateBase* GS = World->GetGameState();
    const int32 CurrentPlayers = (GS ? GS->PlayerArray.Num() : 0);

    if (CurrentPlayers > 0)
    {
        LastNonZeroPlayerTimeSec = Now;
        return; // 사람이 있으면 OK
    }

    // 무인 상태가 grace 이상 지속되면 종료
    if (IdleShutdownGraceSeconds > 0.f && (Now - LastNonZeroPlayerTimeSec) >= IdleShutdownGraceSeconds)
    {
        UE_LOG(GameServerLog, Warning, TEXT("Zero-player idle for %.1fs (>= %.1fs). Shutting down."),
               Now - LastNonZeroPlayerTimeSec, IdleShutdownGraceSeconds);
        TerminateServerProcess();
    }
}
#endif

float UGS_GameInstance::GetMouseSensitivity() const
{
    return MouseSensitivity;
}

void UGS_GameInstance::SetMouseSensitivity(float NewSensitivity)
{
    MouseSensitivity = NewSensitivity;
}

void UGS_GameInstance::SaveSettings()
{
    if (UGS_OptionSettinsSaveGame* SaveGameInstance = Cast<UGS_OptionSettinsSaveGame>(UGameplayStatics::CreateSaveGameObject(UGS_OptionSettinsSaveGame::StaticClass())))
    {
        SaveGameInstance->MouseSensitivity = MouseSensitivity;
        SaveGameInstance->BGMVolume = BGMVolume;
        SaveGameInstance->SFXVolume = SFXVolume;
        SaveGameInstance->LanguageSet = LanguageSet;

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
            SFXVolume = LoadGameInstance->SFXVolume;
            LanguageSet = LoadGameInstance->LanguageSet;
        }
    }
    else
    {
        // 저장 파일이 없는 경우 기본값 설정
        MouseSensitivity = 1.0f;
        BGMVolume = 1.0f;
        SFXVolume = 1.0f;
        LanguageSet = "ko";
    }
    FInternationalization::Get().SetCurrentCulture(LanguageSet);
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

float UGS_GameInstance::GetSFXVolume() const
{
    return SFXVolume;
}

void UGS_GameInstance::SetSFXVolume(float NewVolume)
{
    const float ClampedVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);

    // 값이 변경되지 않았으면 저장하지 않음 (성능 최적화)
    if (FMath::IsNearlyEqual(SFXVolume, ClampedVolume, 0.001f))
    {
        return;
    }

    SFXVolume = ClampedVolume;

    // AudioManager를 통해 실제 볼륨 적용
    if (UGS_AudioManager* AudioManager = GetSubsystem<UGS_AudioManager>())
    {
        AudioManager->SetSFXVolume(SFXVolume);
    }

    SaveSettings();
}

FString UGS_GameInstance::GetLanguageSet() const
{
    return LanguageSet;
}

void UGS_GameInstance::SetLanguageSet(FString CurrCulture)
{
    LanguageSet = CurrCulture;
    FInternationalization::Get().SetCurrentCulture(LanguageSet);
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

        int32 AssignedPort = InGameSession.GetPort();
        UE_LOG(GameServerLog, Log, TEXT("GameSession assigned to port: %d"), AssignedPort);

        AsyncTask(ENamedThreads::GameThread, [=, this]()
        {
            UE_LOG(GameServerLog, Log, TEXT("GameThread: Activating GameSession: %s"), *GameSessionId);
            
            FGameLiftGenericOutcome ActivateOutcome = GameLiftSdkModule->ActivateGameSession();
            if (!ActivateOutcome.IsSuccess())
            {
                UE_LOG(GameServerLog, Fatal, TEXT("Failed to Activate GameSession: %s"),
                       *ActivateOutcome.GetError().m_errorMessage);
                GameLiftSdkModule->ProcessEnding();
                FGenericPlatformMisc::RequestExit(true);
                return;
            }

            UE_LOG(GameServerLog, Log, TEXT("GameThread: GameSession Activated successfully."));
            bGameSessionActive = true;

            if (UWorld* World = GetWorld())
            {
                LastNonZeroPlayerTimeSec = World->GetRealTimeSeconds();
            }
            StartHealthAndIdleMonitor();
        });
    });

    // OnTerminate 콜백 설정
    ProcessParameters->OnTerminate.BindLambda([=]()
    {
        UE_LOG(GameServerLog, Log, TEXT("Game Server Process is terminating"));
        GameLiftSdkModule->ProcessEnding();
        FGenericPlatformMisc::RequestExit(true);
    });

    // OnHealthCheck 콜백 설정
    ProcessParameters->OnHealthCheck.BindLambda([]()
    {
        UE_LOG(GameServerLog, Log, TEXT("Performing Health Check"));
        return true;
    });

    // 로그 파일 경로 지정
    TArray<FString> Logfiles;
    Logfiles.Add(TEXT("C:/game/logs/server.log")); // EC2 인스턴스 내의 경로
    ProcessParameters->logParameters = Logfiles;

    ProcessParameters->port = FURL::UrlConfig.DefaultPort;

#if WITH_GAMELIFT
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

