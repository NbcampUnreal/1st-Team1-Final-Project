#include "System/GameMode/GS_InGameGM.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GS_GameInstance.h"
#include "System/GS_PlayerState.h"
#include "System/GS_PlayerRole.h"
#include "AI/RTS/GS_RTSController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Blueprint/UserWidget.h"
#include "Character/GS_TpsController.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/HUD.h"
#include "UI/Character/GS_HPBoardWidget.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "DungeonEditor/Data/GS_DungeonEditorSaveGame.h"

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
            FString RoleString = UEnum::GetValueAsString(PS->CurrentPlayerRole);
            UE_LOG(LogTemp, Warning, TEXT("Player: %s, Role: %s"), *PS->GetPlayerName(), *RoleString);
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

void AGS_InGameGM::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) // 여기부터 호출 순서대로 나열했음
{
    //Super 함수 호출 X
    if (NewPlayer)
    {
        PendingPlayers.Add(NewPlayer);
        UE_LOG(LogTemp, Log, TEXT("Player %s added to pending list. Waiting for dungeon generation."), *NewPlayer->GetName());
    }
}

void AGS_InGameGM::SpawnDungeonFromArray(const TArray<FDESaveData>& SaveData)
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnDungeonFromArray called with %d objects."), SaveData.Num());
    CachedSaveData = SaveData;
    if (SaveData.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("There is no SaveData. Can't Spawn Dungeon."));
        return;
    }
    // 받아온 데이터를 기반으로 "몬스터"를 제외한 액터 스폰
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (const FDESaveData& ObjectData : SaveData)
        {
            if (TSubclassOf<AActor> ActorClassToSpawn = LoadClass<AActor>(nullptr, *ObjectData.SpawnActorClassPath))
            {
                if (!Cast<AGS_Monster>(ActorClassToSpawn))
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    World->SpawnActor<AActor>(ActorClassToSpawn, ObjectData.SpawnTransform, SpawnParams);
                }
            }
        }
    }

    // 내비메시 재빌드 요청
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (IsValid(NavSystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("Requesting NavMesh rebuild..."));
        // 모든 내비메시를 새로 빌드하도록 요청. 이 작업은 즉시 끝나지 않음
        NavSystem->Build();

        // 빌드가 완료되었는지 0.1초마다 확인하는 타이머 시작
        GetWorld()->GetTimerManager().SetTimer(
            NavMeshBuildTimerHandle,
            this,
            &AGS_InGameGM::CheckNavMeshBuildStatus,
            0.1f,
            true); // 반복 실행
    }
}

void AGS_InGameGM::CheckNavMeshBuildStatus()
{
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    // 내비메시 빌드가 진행 중이 아닌지(=완료되었는지) 확인
    if (IsValid(NavSystem) && !NavSystem->IsNavigationBuildInProgress())
    {
        UE_LOG(LogTemp, Warning, TEXT("NavMesh build is complete!"));
        GetWorld()->GetTimerManager().ClearTimer(NavMeshBuildTimerHandle);
        OnNavMeshBuildComplete();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("... Waiting for NavMesh build to finish ..."));
    }
}


void AGS_InGameGM::OnNavMeshBuildComplete()
{
    if (CachedSaveData.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("There is no CachedSaveData. Can't Spawn Monsters."));
        return;
    }

    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (const FDESaveData& ObjectData : CachedSaveData)
        {
            if (TSubclassOf<AActor> ActorClassToSpawn = LoadClass<AActor>(nullptr, *ObjectData.SpawnActorClassPath))
            {
                if (Cast<AGS_Monster>(ActorClassToSpawn))
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    World->SpawnActor<AActor>(ActorClassToSpawn, ObjectData.SpawnTransform, SpawnParams);
                }
            }
        }
    }
    FTimerHandle DelayedRestartPlayerHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayedRestartPlayerHandle, this, &AGS_InGameGM::DelayedRestartPlayer, 2.f, false);
}

void AGS_InGameGM::DelayedRestartPlayer()
{
    UE_LOG(LogTemp, Log, TEXT("DelayedRestartPlayer Executed!!! Dungeon generation complete. Restarting all players."));
    if (GameState)
    {
        if (PendingPlayers.Num() > 0)
        {
            for (AController* PC : PendingPlayers)
            {
                AActor* FoundStart = FindPlayerStart(PC);
                if (FoundStart)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Found valid PlayerStart '%s' with for %s at location: %s"),
                        *FoundStart->GetName(),
                        *PC->PlayerState->GetPlayerName(),
                        *FoundStart->GetActorLocation().ToString()
                    );
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("FAILED to find any valid PlayerStart for %s. Player will spawn at origin."), *PC->PlayerState->GetPlayerName());
                }

                if (PC)
                {
                    RestartPlayer(PC);
                }
            }
            PendingPlayers.Empty();
        }
    }
}

AActor* AGS_InGameGM::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    UE_LOG(LogTemp, Warning, TEXT("Called FindPlayerStart_Implementation."));

    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        APlayerStart* PlayerStart = *It;
        if (PlayerStart)
        {
            UE_LOG(LogTemp, Warning, TEXT("Found a valid PlayerStart '%s' at location %s via TActorIterator."), *PlayerStart->GetName(), *PlayerStart->GetActorLocation().ToString());
            return PlayerStart;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("OVERRIDDEN FindPlayerStart FAILED to find any PlayerStart actors. Falling back to default behavior."));
    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

 //AActor* AGS_InGameGM::ChoosePlayerStart_Implementation(AController* Player)
 //{
 //    FString PlayerStartTagToFind = TEXT("");

 //    if (Player)
 //    {
 //        AGS_PlayerState* PS = Player->GetPlayerState<AGS_PlayerState>();
 //        if (PS)
 //        {
 //            if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
 //            {
 //                PlayerStartTagToFind = TEXT("SpawnPoint1");
 //            }
 //            else if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
 //            {
 //                PlayerStartTagToFind = TEXT("SpawnPoint1");
 //            }
 //        }
 //    }

 //    AActor* FoundPlayerStart = FindPlayerStart_Implementation(Player, PlayerStartTagToFind);

 //    if (!FoundPlayerStart && !PlayerStartTagToFind.IsEmpty())
 //    {
 //        UE_LOG(LogTemp, Warning, TEXT("Could not find a PlayerStart with tag '%s'. Searching for any available PlayerStart."), *PlayerStartTagToFind);
 //        FoundPlayerStart = FindPlayerStart(Player, ""); // 태그 없이 다시 검색
 //    }

 //    if (FoundPlayerStart)
 //    {
 //        return FoundPlayerStart;
 //    }
 //    return nullptr;
 //} // 여기까지 호출 순서대로 나열. 이후는 모름

void AGS_InGameGM::StartPlay()
{
    Super::StartPlay();

    if (GameState)
    {
        for (APlayerState* PS : GameState->PlayerArray)
        {
            if (APlayerController* PC = PS->GetPlayerController())
            {
                UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM::StartPlay - PlayerController: %s"), *PC->GetClass()->GetName())
                BindToPlayerState(PC);
            }
            else if (AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS))
            {
                GS_PS->OnPlayerAliveStatusChangedDelegate.RemoveAll(this);
                GS_PS->OnPlayerAliveStatusChangedDelegate.AddUObject(this, &AGS_InGameGM::HandlePlayerAliveStatusChanged);
                UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Bound to existing player (No PC) %s"), *GS_PS->GetPlayerName());
                HandlePlayerAliveStatusChanged(GS_PS, GS_PS->bIsAlive);
            }
            AGS_PlayerState* GPS = Cast<AGS_PlayerState>(PS);
            if (GPS && GPS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
            {
                UE_LOG(LogTemp, Warning, TEXT("Guardian Player '%s' found in StartPlay. Spawning dungeon..."), *GPS->GetPlayerName());
                SpawnDungeonFromArray(GPS->ObjectData);
            }
        }
    }
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
        //// [TODO] 플레이어 widget 업데이트
        //AGS_TpsController* TpsPC = Cast<AGS_TpsController>(PlayerController);
        //if (IsValid(TpsPC))
        //{
        //    UE_LOG(LogTemp, Warning, TEXT("@@@@@@@@@@@@@@ TPS PC @@@@@@@@@@@@@@@@@@@@@"));
        //    TpsPC->TestFunction();
        //    
        //    // UUserWidget* Widget = TpsPC->GetPlayerWidget();
        //    // if (Widget)
        //    // {
        //    //     UGS_HPBoardWidget* HPBoardWidget = Cast<UGS_HPBoardWidget>(Widget->GetWidgetFromName(TEXT("WBP_HPBoard")));
        //    //     UE_LOG(LogTemp, Warning, TEXT("@@@@@@@@@@@@@@ valid widget"));
        //    //     if (IsValid(HPBoardWidget))
        //    //     {
        //    //         UE_LOG(LogTemp, Warning, TEXT("@@@@@@@@@@@@@@ valid HPBoard Widget"));
        //    //         HPBoardWidget->InitBoardWidget();
        //    //     }
        //    // }
        //}
        
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

void AGS_InGameGM::OnTimerEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("AGS_InGameGM: OnTimerEnd called (likely by GameState). Seekers Lose."));
    EndGame(EGameResult::GR_SeekersLost);
}

void AGS_InGameGM::EndGame(EGameResult Result)
{
    // 명시적으로 bIsAlive를 다시 한번 확실하게 동기화
    if (GameState)
    {
        for (APlayerState* PS : GameState->PlayerArray)
        {
            if (AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS))
            {
                if (GS_PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
                {
                    GS_PS->SetIsAlive(true);
                    UE_LOG(LogTemp, Warning, TEXT("EndGame: Guardian (%s) bIsAlive state forced to TRUE before travel."), *GS_PS->GetPlayerName());
                }
                else if (GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
                {
                    if (AGS_Character* SeekerPawn = Cast<AGS_Character>(GS_PS->GetPawn()))
                    {
                        if (UGS_StatComp* StatComp = SeekerPawn->GetStatComp())
                        {
                            GS_PS->HandleCurrentHPChanged(StatComp);
                        }
                    }
                }
            }
        }
    }

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

void AGS_InGameGM::NotifyPlayerIsReady(AController* PlayerController)
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

void AGS_InGameGM::StartMatchWhenAllReady()
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
        EndGame(EGameResult::GR_SeekersLost);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("AGS_InGameGM: Not all players are dead yet. (%d players checked)"), PlayerCount);
    }
}
