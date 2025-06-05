#include "System/GameMode/GS_InGameGM.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GS_GameInstance.h"
#include "System/GS_PlayerState.h"
#include "System/GS_GameInstance.h"
#include "System/GS_PlayerRole.h"
#include "AI/RTS/GS_RTSController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "Character/GS_TpsController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/HUD.h"
#include "UI/Character/GS_HPBoardWidget.h"

AGS_InGameGM::AGS_InGameGM()
{
    DefaultPawnClass = nullptr;
	GameStateClass = AGS_InGameGS::StaticClass();
    PlayerStateClass = AGS_PlayerState::StaticClass();
    bUseSeamlessTravel = true;
}

TSubclassOf<APlayerController> AGS_InGameGM::GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC)
{
    if (const auto* PS = PreviousPC ? PreviousPC->GetPlayerState<AGS_PlayerState>() : nullptr)
    {
        return (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? SeekerControllerClass : RTSControllerClass;
    }

    return Super::GetPlayerControllerClassToSpawnForSeamlessTravel(PreviousPC);
}

void AGS_InGameGM::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    if (auto* PC = Cast<APlayerController>(C))
    {
        if (const auto* PS = PC->GetPlayerState<AGS_PlayerState>())
        {
            if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
            {
                PC->ClientSetHUD(SeekerHUDClass);
            }
            else
            {
                PC->ClientSetHUD(RTSHUDClass);
            }
        }
    }
}

UClass* AGS_InGameGM::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    const AGS_PlayerState* PS = InController ? InController->GetPlayerState<AGS_PlayerState>() : nullptr;
    UClass* ResolvedPawnClass = nullptr;

    if (!PawnMappingDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AGS_InGameGM: PawnMappingDataAsset is NOT ASSIGNED in the GameMode Blueprint! Attempting to use super's default."));
        return Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    if (!PS)
    {
        return Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
    {
        UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Guardian player in InGameLevel. Returning nullptr for PawnClass."));
        ResolvedPawnClass = nullptr;
    }
    else if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
    {
        const FAssetToSpawn* FoundAssetSetup = PawnMappingDataAsset->SeekerPawnClasses.Find(PS->CurrentSeekerJob);
        if (FoundAssetSetup && *FoundAssetSetup->PawnClass)
        {
            ResolvedPawnClass = *FoundAssetSetup->PawnClass;
        }
        else
        {
            ResolvedPawnClass = PawnMappingDataAsset->DefaultSeekerPawn;
        }
            
    }

    if (!ResolvedPawnClass && (!PS || PS->CurrentPlayerRole != EPlayerRole::PR_Guardian))
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: Pawn class not resolved (and not an intentionally unpawned Guardian). Attempting to use super's default."));
        ResolvedPawnClass = Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    return ResolvedPawnClass;
}

AActor* AGS_InGameGM::ChoosePlayerStart_Implementation(AController* Player)
{
    FString PlayerStartTagToFind = TEXT("");

    if (Player)
    {
        AGS_PlayerState* PS = Player->GetPlayerState<AGS_PlayerState>();
        if (PS)
        {
            if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
            {
                PlayerStartTagToFind = TEXT("SpawnPoint1");
            }
            else if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
            {
                PlayerStartTagToFind = TEXT("SpawnPoint1");
                
            }
        }
    }

    AActor* FoundPlayerStart = FindPlayerStart_Implementation(Player, PlayerStartTagToFind);

    if (FoundPlayerStart)
    {
        return FoundPlayerStart;
    }

    //Super::ChoosePlayerStart_Implementation(Player) 호출과 유사한 효과를 내지만, 재귀 호출 위험을 줄임
    if (!PlayerStartTagToFind.IsEmpty())
    {
        FoundPlayerStart = FindPlayerStart_Implementation(Player, TEXT(""));

        if (FoundPlayerStart)
        {
            return FoundPlayerStart;
        }
    }
    return nullptr;
}

void AGS_InGameGM::StartPlay()
{
    Super::StartPlay();

    bMatchHasStarted = false;
    bGameEnded = false;

    /*FString BossLevelName = TEXT("BossLevel");
    FString ResultLevelName = TEXT("ResultLevel");
    UGameplayStatics::LoadStreamLevel(this, FName(*BossLevelName), false, false, FLatentActionInfo());
    UGameplayStatics::LoadStreamLevel(this, FName(*ResultLevelName), false, false, FLatentActionInfo());*/

    FTimerHandle TempHandle;
    GetWorldTimerManager().SetTimer(TempHandle, [this]() {
        if (GameState)
        {
            for (APlayerState* PS : GameState->PlayerArray)
            {
                if (APlayerController* PC = PS->GetPlayerController())
                {
                    BindToPlayerState(PC);
                }
                else if (AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS))
                {
                    GS_PS->OnPlayerAliveStatusChangedDelegate.RemoveAll(this);
                    GS_PS->OnPlayerAliveStatusChangedDelegate.AddUObject(this, &AGS_InGameGM::HandlePlayerAliveStatusChanged);
                    UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Bound to existing player (No PC) %s"), *GS_PS->GetPlayerName());
                    HandlePlayerAliveStatusChanged(GS_PS, GS_PS->bIsAlive);
                }
            }
            GetWorldTimerManager().SetTimer(MatchStartTimerHandle, this, &AGS_InGameGM::StartMatchCheck, 2.0f, false);
        }
    }, 0.2f, false);
}

void AGS_InGameGM::Logout(AController* Exiting)
{
    if (Exiting)
    {
        AGS_PlayerState* GS_PS = Exiting->GetPlayerState<AGS_PlayerState>();
        if (GS_PS)
        {
            GS_PS->OnPlayerAliveStatusChangedDelegate.RemoveAll(this);
            UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Unbound from logging out player %s"), *GS_PS->GetPlayerName());
        }
    }
    Super::Logout(Exiting);

    // 플레이어가 나갔을 때도 생존자 확인
    CheckAllPlayersDead();
}

void AGS_InGameGM::BindToPlayerState(APlayerController* PlayerController)
{
    if (PlayerController)
    {
        // [TODO] 플레이어 widget 업데이트
        AGS_TpsController* TpsPC = Cast<AGS_TpsController>(PlayerController);
        if (IsValid(TpsPC))
        {
            TpsPC->TestFunction();
            
            // UUserWidget* Widget = TpsPC->GetPlayerWidget();
            // if (Widget)
            // {
            //     UGS_HPBoardWidget* HPBoardWidget = Cast<UGS_HPBoardWidget>(Widget->GetWidgetFromName(TEXT("WBP_HPBoard")));
            //     UE_LOG(LogTemp, Warning, TEXT("@@@@@@@@@@@@@@ valid widget"));
            //     if (IsValid(HPBoardWidget))
            //     {
            //         UE_LOG(LogTemp, Warning, TEXT("@@@@@@@@@@@@@@ valid HPBoard Widget"));
            //         HPBoardWidget->InitBoardWidget();
            //     }
            // }
        }
        
        AGS_PlayerState* GS_PS = PlayerController->GetPlayerState<AGS_PlayerState>();
        if (GS_PS)
        {
            GS_PS->OnPlayerAliveStatusChangedDelegate.RemoveAll(this);
            GS_PS->OnPlayerAliveStatusChangedDelegate.AddUObject(this, &AGS_InGameGM::HandlePlayerAliveStatusChanged);
            UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Bound to player %s"), *GS_PS->GetPlayerName());
            HandlePlayerAliveStatusChanged(GS_PS, GS_PS->bIsAlive);
        }
    }
}

void AGS_InGameGM::StartMatchCheck()
{
    UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Match checks started. bMatchHasStarted = true"));
    bMatchHasStarted = true;
    CheckAllPlayersDead(); //이거 지워도 상관 없음
}

void AGS_InGameGM::OnTimerEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: OnTimerEnd called (likely by GameState). Seekers Lose."));
    EndGame(EGameResult::GR_SeekersLost);
}

void AGS_InGameGM::EndGame(EGameResult Result)
{
    if (bGameEnded)
    {
        return;
    }
    bGameEnded = true;

    FString NextLevelName;

    if (Result == EGameResult::GR_SeekersLost)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: Seekers Lost. Traveling to EndingLevel."));
        SetGameResultOnAllPlayers(EGameResult::GR_SeekersLost);
        NextLevelName = TEXT("ResultLevel");
	}
    else if (Result == EGameResult::GR_InProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: Not All Seekers dead. Traveling to BossLevel."));
        SetGameResultOnAllPlayers(EGameResult::GR_InProgress);
        NextLevelName = TEXT("BossLevel");

        AGS_InGameGS* InGameGS = GetGameState<AGS_InGameGS>();
        if (InGameGS)
        {
			float RemainingTime = FMath::Max(0.0f, InGameGS->TotalGameTime - InGameGS->CurrentTime);
			UGS_GameInstance* GI = Cast<UGS_GameInstance>(GetGameInstance());
            if (GI)
            {
				GI->RemainingTime = RemainingTime;
            }
        }
    }

    if (!NextLevelName.IsEmpty())
    {
        FTimerHandle TravelDelayHandle;
        GetWorldTimerManager().SetTimer(TravelDelayHandle, [this, NextLevelName]() {
            GetWorld()->ServerTravel(NextLevelName + "?listen", true);
        }, 3.f, false);
    }
}

void AGS_InGameGM::SetGameResultOnAllPlayers(EGameResult Result)
{
    if (!GameState) return;

    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS))
        {
            GS_PS->CurrentGameResult = Result;
        }
    }
}

void AGS_InGameGM::HandlePlayerAliveStatusChanged(AGS_PlayerState* PlayerState, bool bIsAlive)
{
    UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Player %s alive status changed to %s"),
        *PlayerState->GetPlayerName(),
        bIsAlive ? TEXT("True") : TEXT("False"));

    if (!bIsAlive)
    {
        CheckAllPlayersDead();
    }
}

void AGS_InGameGM::CheckAllPlayersDead()
{
    if (!GameState) return;

    bool bAllPlayersDead = true;
    int32 PlayerCount = 0;

    for (APlayerState* PS : GameState->PlayerArray)
    {
        AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
        if (GS_PS && GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
        {
            PlayerCount++;
            if (GS_PS->bIsAlive)
            {
                bAllPlayersDead = false;
                break;
            }
        }
    }

    if (PlayerCount > 0 && bAllPlayersDead)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: All relevant players are dead!"));
        if (bMatchHasStarted)
        {
            EndGame(EGameResult::GR_SeekersLost);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: All dead check triggered, but match not started yet. Ignoring EndGame."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Not all players are dead yet. (%d players checked)"), PlayerCount);
    }
}