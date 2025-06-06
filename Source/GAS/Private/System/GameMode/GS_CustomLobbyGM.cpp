#include "System/GameMode/GS_CustomLobbyGM.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "System/GS_PlayerState.h"
#include "System/GameState/GS_CustomLobbyGS.h"
#include "System/GS_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GS_SpawnSlot.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"

AGS_CustomLobbyGM::AGS_CustomLobbyGM()
{
    DefaultPawnClass = nullptr;
	PlayerStateClass = AGS_PlayerState::StaticClass();
	PlayerControllerClass = AGS_CustomLobbyPC::StaticClass();
	GameStateClass = AGS_CustomLobbyGS::StaticClass();
    bUseSeamlessTravel = true;
}

void AGS_CustomLobbyGM::BeginPlay()
{
    Super::BeginPlay();

    CollectSpawnSlots();
}

void AGS_CustomLobbyGM::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AGS_PlayerState* PS = NewPlayer->GetPlayerState<AGS_PlayerState>();
	if (PS)
	{
		PlayerReadyStates.Add(PS, false);
		PS->InitializeDefaults();
		UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s logged in. Total players: %d"), *PS->GetPlayerName(), PlayerReadyStates.Num());
	}

    UpdateLobbyPawns();
}

void AGS_CustomLobbyGM::Logout(AController* Exiting)
{
	AGS_PlayerState* PS = Cast<AGS_PlayerState>(Exiting->PlayerState);
	if (PS)
	{
		PlayerReadyStates.Remove(PS);
		UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s logged out. Total players: %d"), *PS->GetPlayerName(), PlayerReadyStates.Num());
		CheckAllPlayersReady();
	}

    if (PS && SpawnedLobbyPawns.Contains(PS))
    {
        APawn* PawnToDestroy = SpawnedLobbyPawns[PS];
        if (PawnToDestroy)
        {
            PawnToDestroy->Destroy();
        }
        SpawnedLobbyPawns.Remove(PS);
    }

    Super::Logout(Exiting);

	UpdateLobbyPawns();
}

void AGS_CustomLobbyGM::UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady)
{
	if (Player)
	{
		bool* FoundStatus = PlayerReadyStates.Find(Player);
		if (FoundStatus)
		{
			*FoundStatus = bIsReady;
			UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s is now %s"), *Player->GetPlayerName(), bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
			CheckAllPlayersReady();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyGM: Player %s not found in PlayerReadyStates"), *Player->GetPlayerName());
		}
	}
}

void AGS_CustomLobbyGM::CheckAllPlayersReady() //가디언 1명 아니면 시작 안 되는 로직 추가해야함
{
    AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>();
    if (!LGS)
    {
        UE_LOG(LogTemp, Error, TEXT("LobbyGM: AGS_CustomLobbyGS is null in CheckAllPlayersReady."));
        return;
    }

    const int32 CurrentPlayerCount = LGS->PlayerArray.Num();
    UE_LOG(LogTemp, Verbose, TEXT("LobbyGM: Checking ready status for %d players in GameState. Min players: %d. Players in local map: %d"),
        CurrentPlayerCount, MinPlayersToStart, PlayerReadyStates.Num());



    // 최소 시작 인원 확인
    if (CurrentPlayerCount < MinPlayersToStart)
    {
        UE_LOG(LogTemp, Log, TEXT("LobbyGM: Not enough players to start (%d / %d)."), CurrentPlayerCount, MinPlayersToStart);
        return;
    }

    bool bAllReady = true;
    for (APlayerState* Player : LGS->PlayerArray)
    {
        if (!Player)
        {
            bAllReady = false;
            break;
        }

        const bool* PlayerStatusInMap = PlayerReadyStates.Find(Player);
        if (PlayerStatusInMap == nullptr)
        {
            bAllReady = false;
            break;
        }

        if (*PlayerStatusInMap == false)
        {
            bAllReady = false;
            UE_LOG(LogTemp, Log, TEXT("LobbyGM: Some Player 'Not Ready'."));
            break;
        }
    }

    if (bAllReady && CurrentPlayerCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("LobbyGM: All %d players are ready! Traveling to %s..."), CurrentPlayerCount, *NextLevelName.ToString());
        UWorld* World = GetWorld();
        if (World)
        {
            UE_LOG(LogTemp, Warning, TEXT("LobbyGM: ServerTravel Start ***"));
            bUseSeamlessTravel = true;
            World->ServerTravel(NextLevelName.ToString() + "?listen", true);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("LobbyGM: World is null, cannot ServerTravel."));
        }
    }
    else
    {
        if (CurrentPlayerCount > 0) // 플레이어가 있을 때만 "아직 준비 안됨" 로그 출력
        {
            UE_LOG(LogTemp, Log, TEXT("LobbyGM: Not all players are ready yet or no players connected."));
        }
    }
}

void AGS_CustomLobbyGM::HandlePlayerStateUpdated(AGS_PlayerState* UpdatedPlayerState)
{
    if (UpdatedPlayerState)
    {
        UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player State updated for %s. Refreshing lobby."), *UpdatedPlayerState->GetPlayerName());
    }
    UpdateLobbyPawns();
}

void AGS_CustomLobbyGM::CollectSpawnSlots()
{
    GuardianSlots.Empty();
    SeekerSlots.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGS_SpawnSlot::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        AGS_SpawnSlot* Slot = Cast<AGS_SpawnSlot>(Actor);
        if (Slot)
        {
            if (Slot->GetRole() == EPlayerRole::PR_Guardian)
            {
                GuardianSlots.Add(Slot);
            }
            else if (Slot->GetRole() == EPlayerRole::PR_Seeker)
            {
                SeekerSlots.Add(Slot);
            }
        }
    }

    SeekerSlots.Sort([](const AGS_SpawnSlot& A, const AGS_SpawnSlot& B) {
        return A.GetSlotIndex() < B.GetSlotIndex();
    });

    UE_LOG(LogTemp, Log, TEXT("LobbyGM: Collected %d Guardian slots and %d Seeker slots."), GuardianSlots.Num(), SeekerSlots.Num());
}

void AGS_CustomLobbyGM::UpdateLobbyPawns()
{
    AGS_CustomLobbyGS* GS = GetGameState<AGS_CustomLobbyGS>();
    if (!PawnMappingData)
    {
        UE_LOG(LogTemp, Error, TEXT("AGS_CustomLobbyGM::UpdateLobbyPawns - PawnMappingData is not set!"));
        return;
    }
    if (!GS) return;

    // 1. 기존에 스폰된 모든 폰을 먼저 파괴합니다
    for (auto& Pair : SpawnedLobbyPawns)
    {
        if (Pair.Value)
        {
            Pair.Value->Destroy();
        }
    }
    SpawnedLobbyPawns.Empty();

    // 2. 역할을 선택한 플레이어들을 분류합니다
    TArray<AGS_PlayerState*> Guardians;
    TArray<AGS_PlayerState*> Seekers;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AGS_PlayerState* Player = Cast<AGS_PlayerState>(PS);
        if (Player)
        {
            if (Player->CurrentPlayerRole == EPlayerRole::PR_Guardian) Guardians.Add(Player);
            else if (Player->CurrentPlayerRole == EPlayerRole::PR_Seeker) Seekers.Add(Player);
        }
    }

    // 3. Guardian 폰을 스폰합니다
    for (int32 i = 0; i < Guardians.Num() && i < GuardianSlots.Num(); ++i)
    {
        AGS_PlayerState* GuardianPS = Guardians[i];
        AGS_SpawnSlot* Slot = GuardianSlots[i];

        const FAssetToSpawn* SpawnInfo = PawnMappingData->GuardianPawnClasses.Find(GuardianPS->CurrentGuardianJob);
        if (SpawnInfo && SpawnInfo->PawnClass)
        {
            TSubclassOf<APawn> PawnToSpawn = SpawnInfo->PawnClass;
            APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnToSpawn, Slot->GetActorTransform());
            SpawnedLobbyPawns.Add(GuardianPS, NewPawn);
        }
    }

    // 4. Seeker 폰들을 스폰합니다
    for (int32 i = 0; i < Seekers.Num() && i < SeekerSlots.Num(); ++i)
    {
        AGS_PlayerState* SeekerPS = Seekers[i];
        AGS_SpawnSlot* Slot = SeekerSlots[i];

        const FAssetToSpawn* SpawnInfo = PawnMappingData->SeekerPawnClasses.Find(SeekerPS->CurrentSeekerJob);
        if (SpawnInfo && SpawnInfo->PawnClass)
        {
            TSubclassOf<APawn> PawnToSpawn = SpawnInfo->PawnClass;
            APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnToSpawn, Slot->GetActorTransform());
            SpawnedLobbyPawns.Add(SeekerPS, NewPawn);
        }
    }
}
