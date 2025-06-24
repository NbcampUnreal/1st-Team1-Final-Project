#include "System/GameMode/GS_CustomLobbyGM.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "System/GS_PlayerState.h"
#include "System/GameState/GS_CustomLobbyGS.h"
#include "System/GS_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GS_SpawnSlot.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Animation/AnimInstance.h"
#include "Animation/Character/GS_LobbyAnimInstance.h"
#include "Character/Player/GS_LobbyDisplayActor.h"

AGS_CustomLobbyGM::AGS_CustomLobbyGM()
{
    DefaultPawnClass = nullptr;
	PlayerStateClass = AGS_PlayerState::StaticClass();
	PlayerControllerClass = AGS_CustomLobbyPC::StaticClass();
	GameStateClass = AGS_CustomLobbyGS::StaticClass();
    bUseSeamlessTravel = true;
}

void AGS_CustomLobbyGM::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (!ErrorMessage.IsEmpty()) return;

    if (GameState)
    {
        const bool bIsFromInvite = Options.Contains(TEXT("bIsFromInvite=true"));
        if (GameState->PlayerArray.Num() >= 1 && !bIsFromInvite)
        {
            ErrorMessage = TEXT("Server is full.");
        }
	}
    else
    {
        ErrorMessage = TEXT("Server is not ready.");
    }
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
        PS->InitializeDefaults();

        AGS_SpawnSlot* AvailableSlot = FindAvailableSlotForPlayer(PS);
        if (AvailableSlot)
        {
            UE_LOG(LogTemp, Warning, TEXT("--> GM::PostLogin - %s 에게 스폰 슬롯 %d 번을 할당합니다."), *PS->GetPlayerName(), AvailableSlot->GetSlotIndex());
            PlayerToSlotMap.Add(PS, AvailableSlot);
            if (AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>())
            {
                LGS->AddPlayerToList(PS, AvailableSlot->GetSlotIndex());
                UE_LOG(LogTemp, Warning, TEXT("--> GM::PostLogin - %s 정보를 GameState의 PlayerList에 추가했습니다."), *PS->GetPlayerName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("--> GM::PostLogin - %s 에게 할당할 수 있는 빈 스폰 슬롯이 없습니다!"), *PS->GetPlayerName());
        }
    }
}

void AGS_CustomLobbyGM::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);

    AGS_PlayerState* PS = NewPlayer->GetPlayerState<AGS_PlayerState>();
    if (PS)
    {
        PS->InitializeDefaults();

        AGS_SpawnSlot* AvailableSlot = FindAvailableSlotForPlayer(PS);
        if (AvailableSlot)
        {
            PlayerToSlotMap.Add(PS, AvailableSlot);
            if (AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>())
            {
                LGS->AddPlayerToList(PS, AvailableSlot->GetSlotIndex());
            }
        }
    }
}

void AGS_CustomLobbyGM::Logout(AController* Exiting)
{
    AGS_PlayerState* PS = Cast<AGS_PlayerState>(Exiting->PlayerState);
    if (PS)
    {
        PlayerToSlotMap.Remove(PS);
        if (AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>())
        {
            LGS->RemovePlayerFromList(PS);
        }

        CheckAllPlayersReady();
    }

    Super::Logout(Exiting);
}

void AGS_CustomLobbyGM::UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady)
{
    AGS_PlayerState* GSPlayer = Cast<AGS_PlayerState>(Player);
    if (GSPlayer)
    {
        if (AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>())
        {
            LGS->UpdatePlayerInList(GSPlayer);
        }
        CheckAllPlayersReady();
    }
}

void AGS_CustomLobbyGM::CheckAllPlayersReady()
{
    AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>();
    if (!LGS)
    {
        UE_LOG(LogTemp, Error, TEXT("LobbyGM: AGS_CustomLobbyGS is null in CheckAllPlayersReady."));
        return;
    }

    const int32 CurrentPlayerCount = LGS->PlayerArray.Num();

    // 최소 시작 인원 확인
    if (CurrentPlayerCount < MinPlayersToStart)
    {
        UE_LOG(LogTemp, Log, TEXT("LobbyGM: Not enough players to start (%d / %d)."), CurrentPlayerCount, MinPlayersToStart);
        return;
    }

    bool bAllReady = true;
    int32 GuardianCount = 0;
    for (APlayerState* Player : LGS->PlayerArray)
    {
        if (!Player)
        {
            bAllReady = false;
            break;
        }

        AGS_PlayerState* GSPlayerState = Cast<AGS_PlayerState>(Player);
        if (GSPlayerState)
        {
            if (GSPlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
            {
                GuardianCount++;
            }
        }

        const bool* PlayerStatusInMap = PlayerReadyStates.Find(Player);
        if (PlayerStatusInMap == nullptr || *PlayerStatusInMap == false)
        {
            bAllReady = false;
            break;
        }
    }

    if (!bAllReady || GuardianCount != 1)
    {
        return;
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
    }
}

void AGS_CustomLobbyGM::HandlePlayerStateUpdated(AGS_PlayerState* UpdatedPlayerState)
{
    if (!UpdatedPlayerState) return;

    PlayerToSlotMap.Remove(UpdatedPlayerState);

    AGS_SpawnSlot* NewAvailableSlot = FindAvailableSlotForPlayer(UpdatedPlayerState);
    if (NewAvailableSlot)
    {
        PlayerToSlotMap.Add(UpdatedPlayerState, NewAvailableSlot);
    }

    if (AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>())
    {
        LGS->UpdatePlayerInList(UpdatedPlayerState, NewAvailableSlot ? NewAvailableSlot->GetSlotIndex() : -1);
    }
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

AGS_SpawnSlot* AGS_CustomLobbyGM::FindAvailableSlotForPlayer(AGS_PlayerState* PlayerState)
{
    if (!PlayerState) return nullptr;

    TArray<AGS_SpawnSlot*>& SlotsToSearch = (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian) ? GuardianSlots : SeekerSlots;

    TSet<AGS_SpawnSlot*> UsedSlots;
    for (const auto& Pair : PlayerToSlotMap)
    {
        if (Pair.Value)
        {
            UsedSlots.Add(Pair.Value.Get());
        }
    }

    for (AGS_SpawnSlot* Slot : SlotsToSearch)
    {
        if (!UsedSlots.Contains(Slot))
        {
            return Slot; // 비어있는 첫 번째 슬롯 반환
        }
    }
    return nullptr;
}