#include "System/GS_BaseGM.h"
#include "System/GS_GameInstance.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "AI/RTS/GS_RTSController.h"
#include "Character/GS_TpsController.h"

void AGS_BaseGM::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->OnPlayerCountChanged.Broadcast();
    }
}

void AGS_BaseGM::Logout(AController* Exiting)
{
    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->OnPlayerCountChanged.Broadcast();
    }

    Super::Logout(Exiting);
}

void AGS_BaseGM::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearAllTimersForObject(this);

    Super::EndPlay(EndPlayReason);
}

void AGS_BaseGM::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    GameplayReadyPlayers.Empty();

    UE_LOG(LogTemp, Log, TEXT("Engine is ready. Telling clients to prepare for gameplay."));

    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (APlayerController* PC = PS->GetPlayerController())
        {
            // 컨트롤러의 특정 함수를 호출 (예: Client_PrepareForMatchStart)
            // 이 함수는 클라이언트에서 UI를 띄우거나 인트로 연출을 시작
            if (AGS_TpsController* TpsPC = Cast<AGS_TpsController>(PC))
            {
                UE_LOG(LogTemp, Warning, TEXT("씨커 HandleMatchHasStarted() 호출"));
                TpsPC->Client_PrepareForMatchStart();
            }
            else if (AGS_RTSController* GuardianPC = Cast<AGS_RTSController>(PC))
            {
                UE_LOG(LogTemp, Warning, TEXT("가디언 HandleMatchHasStarted() 호출"));
                GuardianPC->Client_PrepareForMatchStart();
            }
        }
    }
}

void AGS_BaseGM::NotifyPlayerIsReady(AController* PlayerController)
{
    if (!PlayerController || !PlayerController->PlayerState) return;

    GameplayReadyPlayers.AddUnique(PlayerController->PlayerState);
    UE_LOG(LogTemp, Log, TEXT("Player %s is gameplay-ready. Total: %d/%d"),
        *PlayerController->PlayerState->GetPlayerName(), GameplayReadyPlayers.Num(), GameState->PlayerArray.Num());

    if (GameplayReadyPlayers.Num() == GameState->PlayerArray.Num())
    {
        StartMatchWhenAllReady();
    }
}

void AGS_BaseGM::StartMatchWhenAllReady()
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
