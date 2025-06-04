#include "System/GameMode/GS_BossLevelGM.h"
#include "System/GameState/GS_BossLevelGS.h"
#include "System/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerRole.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AGS_BossLevelGM::AGS_BossLevelGM()
{
	PlayerStateClass = AGS_PlayerState::StaticClass();
	bUseSeamlessTravel = true;
}

TSubclassOf<APlayerController> AGS_BossLevelGM::GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC)
{
	if (const auto* PS = PreviousPC ? PreviousPC->GetPlayerState<AGS_PlayerState>() : nullptr)
	{
		return (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? SeekerControllerClass : GuardianControllerClass;
	}

	return Super::GetPlayerControllerClassToSpawnForSeamlessTravel(PreviousPC);
}

UClass* AGS_BossLevelGM::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    const AGS_PlayerState* PS = InController ? InController->GetPlayerState<AGS_PlayerState>() : nullptr;
    UClass* ResolvedPawnClass = nullptr;

    if (!PawnMappingDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("AGS_BossLevelGM: PawnMappingDataAsset is NOT ASSIGNED in the GameMode Blueprint! Attempting to use super's default."));
        return Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    if (PS)
    {
        if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
        {
            if (PS->CurrentGuardianJob != EGuardianJob::End)
            {
                const TSubclassOf<APawn>* FoundPawnClass = PawnMappingDataAsset->GuardianPawnClasses.Find(PS->CurrentGuardianJob);
                if (FoundPawnClass && *FoundPawnClass)
                {
                    ResolvedPawnClass = *FoundPawnClass;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AGS_BossLevelGM: Guardian PawnClass not found in DataAsset for job %s. Using default guardian pawn from DataAsset."), *UEnum::GetValueAsString(PS->CurrentGuardianJob));
                    ResolvedPawnClass = PawnMappingDataAsset->DefaultGuardianPawn;
                }
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("AGS_BossLevelGM: Guardian job is 'End'. Using default guardian pawn from DataAsset."));
                ResolvedPawnClass = PawnMappingDataAsset->DefaultGuardianPawn;
            }
        }
        else if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
        {
            if (PS->CurrentSeekerJob != ESeekerJob::End)
            {
                const TSubclassOf<APawn>* FoundPawnClass = PawnMappingDataAsset->SeekerPawnClasses.Find(PS->CurrentSeekerJob);
                if (FoundPawnClass && *FoundPawnClass)
                {
                    ResolvedPawnClass = *FoundPawnClass;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AGS_BossLevelGM: Seeker PawnClass not found in DataAsset for job: %s. Using default seeker pawn from DataAsset."), *UEnum::GetValueAsString(PS->CurrentSeekerJob));
                    ResolvedPawnClass = PawnMappingDataAsset->DefaultSeekerPawn;
                }
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("AGS_BossLevelGM: Seeker job is 'End'. Using default seeker pawn from DataAsset."));
                ResolvedPawnClass = PawnMappingDataAsset->DefaultSeekerPawn;
            }
        }
    }

    if (!ResolvedPawnClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_BossLevelGM: Pawn class not resolved by role/job logic or PS is null. Attempting to use super's default."));
        ResolvedPawnClass = Super::GetDefaultPawnClassForController_Implementation(InController);
    }

    return ResolvedPawnClass;
}

AActor* AGS_BossLevelGM::ChoosePlayerStart_Implementation(AController* Player)
{
    FString PlayerStartTagToFind = TEXT("");

    if (Player)
    {
        AGS_PlayerState* PS = Player->GetPlayerState<AGS_PlayerState>();
        if (PS)
        {
            if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
            {
                PlayerStartTagToFind = TEXT("SeekerStart");
            }
            else if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
            {
                PlayerStartTagToFind = TEXT("GuardianStart");
            }
        }
    }

    AActor* FoundPlayerStart = FindPlayerStart_Implementation(Player, PlayerStartTagToFind);

    if (FoundPlayerStart)
    {
        return FoundPlayerStart;
    }
    else
    {
        return Super::ChoosePlayerStart_Implementation(Player);
    }
}

void AGS_BossLevelGM::StartPlay()
{
    Super::StartPlay();

    bMatchHasStarted = false;
    bGameEnded = false;

    /*FString ResultLevelName = TEXT("ResultLevel");
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
                    GS_PS->OnPlayerAliveStatusChangedDelegate.AddUObject(this, &AGS_BossLevelGM::HandlePlayerAliveStatusChanged);
                    UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Bound to existing player (No PC) %s"), *GS_PS->GetPlayerName());
                    HandlePlayerAliveStatusChanged(GS_PS, GS_PS->bIsAlive);
                }
            }
            GetWorldTimerManager().SetTimer(MatchStartTimerHandle, this, &AGS_BossLevelGM::StartMatchCheck, 2.0f, false);
        }
    }, 0.2f, false);
}

void AGS_BossLevelGM::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

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

void AGS_BossLevelGM::BindToPlayerState(APlayerController* PlayerController)
{
    if (PlayerController)
    {
        AGS_PlayerState* GS_PS = PlayerController->GetPlayerState<AGS_PlayerState>();
        if (GS_PS)
        {
            GS_PS->OnPlayerAliveStatusChangedDelegate.RemoveAll(this);
            GS_PS->OnPlayerAliveStatusChangedDelegate.AddUObject(this, &AGS_BossLevelGM::HandlePlayerAliveStatusChanged);
            UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Bound to player %s"), *GS_PS->GetPlayerName());
            HandlePlayerAliveStatusChanged(GS_PS, GS_PS->bIsAlive);
        }
    }
}

void AGS_BossLevelGM::StartMatchCheck()
{
    UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Match checks started. bMatchHasStarted = true"));
    bMatchHasStarted = true;
    CheckAllPlayersDead(); //이거 지워도 상관 없음
}

void AGS_BossLevelGM::OnTimerEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: OnTimerEnd called (likely by GameState). Seekers Lose."));
    EndGame(EGameResult::GR_SeekersLost);
}

void AGS_BossLevelGM::EndGame(EGameResult Result)
{
    if (bGameEnded)
    {
        return;
    }
    bGameEnded = true;

    FString NextLevelName = TEXT("ResultLevel");

    if (Result == EGameResult::GR_SeekersLost)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: Seekers Lost. Traveling to EndingLevel."));
        SetGameResultOnAllPlayers(EGameResult::GR_SeekersLost);
    }
    else if (Result == EGameResult::GR_SeekersWon)
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: Not All Seekers dead. Traveling to BossLevel."));
        SetGameResultOnAllPlayers(EGameResult::GR_SeekersWon);
    }

    if (!NextLevelName.IsEmpty())
    {
        FTimerHandle TravelDelayHandle;
        GetWorldTimerManager().SetTimer(TravelDelayHandle, [this, NextLevelName]() {
            GetWorld()->ServerTravel(NextLevelName + "?listen", true);
        }, 3.f, false);
    }
}

void AGS_BossLevelGM::SetGameResultOnAllPlayers(EGameResult Result)
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

void AGS_BossLevelGM::HandlePlayerAliveStatusChanged(AGS_PlayerState* PlayerState, bool bIsAlive)
{
    UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Player %s alive status changed to %s"),
        *PlayerState->GetPlayerName(),
        bIsAlive ? TEXT("True") : TEXT("False"));

    if (!bIsAlive)
    {
        CheckAllPlayersDead();
    }
}

void AGS_BossLevelGM::CheckAllPlayersDead()
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