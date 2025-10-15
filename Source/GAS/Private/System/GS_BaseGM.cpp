#include "System/GS_BaseGM.h"
#include "System/GS_GameInstance.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "AI/RTS/GS_RTSController.h"
#include "Character/GS_TpsController.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#endif

void AGS_BaseGM::Logout(AController* Exiting)
{
#if WITH_GAMELIFT
    if (Exiting && Exiting->PlayerState)
    {
        UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>();
        if (GI)
        {
            FString PlayerSessionId = GI->RemoveAndGetPlayerSession(Exiting->PlayerState->GetUniqueId());
            if (!PlayerSessionId.IsEmpty())
            {
                FGameLiftServerSDKModule* GameLiftSdkModule = FModuleManager::GetModulePtr<FGameLiftServerSDKModule>(TEXT("GameLiftServerSDK"));
                if (GameLiftSdkModule)
                {
                    GameLiftSdkModule->RemovePlayerSession(PlayerSessionId);
                    UE_LOG(LogTemp, Log, TEXT("CustomLobbyGM: Removed PlayerSession '%s' for player '%s'"), *PlayerSessionId, *Exiting->PlayerState->GetPlayerName());
                }
            }
        }
    }
#endif
    
    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->CheckIfLastPlayerAndTerminate();
    }

    Super::Logout(Exiting);
}

void AGS_BaseGM::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearAllTimersForObject(this);

    Super::EndPlay(EndPlayReason);
}

void AGS_BaseGM::NotifyPlayerIsReady(AController* PlayerController)
{
    if (!PlayerController || !PlayerController->PlayerState)
    {
        return;
    }

    ReadyPlayers.Add(PlayerController->PlayerState);
    UE_LOG(LogTemp, Log, TEXT("Player %s is ready. Total ready: %d/%d"), *PlayerController->PlayerState->GetPlayerName(), ReadyPlayers.Num(), GameState->PlayerArray.Num());

    if (ReadyPlayers.Num() == GameState->PlayerArray.Num())
    {
        StartMatchWhenAllReady();
    }
}

void AGS_BaseGM::StartMatchWhenAllReady() // Travel 시작해주는 함수 아니고 Travel 직후 매치 시작 알림 함수임.
{
    UE_LOG(LogTemp, Warning, TEXT("All players are ready! Broadcasting to clients to start the match."));

    // 모든 플레이어 컨트롤러에게 게임 시작을 알리는 Client RPC 호출
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        if (PlayerState)
        {
            if (APlayerController* PC = PlayerState->GetPlayerController())
            {
                if (AGS_TpsController* SeekerPC = Cast<AGS_TpsController>(PC))
                {
                    SeekerPC->Client_StartGame();
                }
                else if (AGS_RTSController* GuardianPC = Cast<AGS_RTSController>(PC))
                {
                    GuardianPC->Client_StartGame();
                }
            }
        }
    }
}
