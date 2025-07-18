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
        PlayerReadyStates.Add(PS, false);
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

void AGS_CustomLobbyGM::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);

    AGS_PlayerState* PS = NewPlayer->GetPlayerState<AGS_PlayerState>();
    if (PS)
    {
        PlayerReadyStates.Add(PS, false);
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

void AGS_CustomLobbyGM::HandlePlayerReadyInLobby(APlayerController* PlayerController)
{
    if (!PlayerController) return;

    AGS_PlayerState* PS = PlayerController->GetPlayerState<AGS_PlayerState>();
    if (PS)
    {
        if (TObjectPtr<AGS_SpawnSlot>* FoundSlot = PlayerToSlotMap.Find(PS))
        {
            if (!SpawnedLobbyActors.Contains(PS))
            {
                SpawnLobbyActorForPlayer(PS, *FoundSlot);
            }
        }
    }
}

void AGS_CustomLobbyGM::Logout(AController* Exiting)
{
    AGS_PlayerState* PS = Cast<AGS_PlayerState>(Exiting->PlayerState);
    if (PS)
    {
        PlayerReadyStates.Remove(PS);
        PlayerToSlotMap.Remove(PS);
        DestroyLobbyActorForPlayer(PS);
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
        if (bool* PlayerStatus = PlayerReadyStates.Find(GSPlayer))
        {
            *PlayerStatus = bIsReady;
        }
        if (AGS_CustomLobbyGS* LGS = GetGameState<AGS_CustomLobbyGS>())
        {
            LGS->UpdatePlayerInList(GSPlayer);
        }
        if (TObjectPtr<AGS_LobbyDisplayActor>* FoundActor = SpawnedLobbyActors.Find(GSPlayer))
        {
            if (AGS_LobbyDisplayActor* LobbyActor = FoundActor->Get())
            {
                LobbyActor->bIsReady = bIsReady;
            }
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
        for (APlayerState* PS : GameState->PlayerArray)
        {
            AGS_PlayerState* GPS = Cast<AGS_PlayerState>(PS);
            if (GPS && GPS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
            {
                if (AGS_CustomLobbyPC* GuardianPC = Cast<AGS_CustomLobbyPC>(GPS->GetPlayerController()))
                {
                    GuardianPC->Client_RequestLoadAndSendData();
                }
                break;
            }
        }
        
        UWorld* World = GetWorld();
        if (World)
        {
            UE_LOG(LogTemp, Warning, TEXT("LobbyGM: ServerTravel Start ***"));
            FTimerHandle TravelDelayHandle;
            World->GetTimerManager().SetTimer(TravelDelayHandle, this, &AGS_CustomLobbyGM::DoServerTravel, 2.0f, false);
        }
    }
}

void AGS_CustomLobbyGM::DoServerTravel()
{
    UWorld* World = GetWorld();
    if (World)
    {
        bUseSeamlessTravel = true;
        World->ServerTravel(NextLevelName.ToString() + "?listen", true);
    }
}

void AGS_CustomLobbyGM::HandlePlayerStateUpdated(AGS_PlayerState* UpdatedPlayerState)
{
    if (!UpdatedPlayerState) return;

    AGS_SpawnSlot* OldSlot = nullptr;
    if (PlayerToSlotMap.Contains(UpdatedPlayerState))
    {
        OldSlot = PlayerToSlotMap[UpdatedPlayerState];
    }

    PlayerToSlotMap.Remove(UpdatedPlayerState);


    AGS_SpawnSlot* NewAvailableSlot = FindAvailableSlotForPlayer(UpdatedPlayerState);
    if (NewAvailableSlot)
    {
        PlayerToSlotMap.Add(UpdatedPlayerState, NewAvailableSlot);
    }

    DestroyLobbyActorForPlayer(UpdatedPlayerState);
    if (NewAvailableSlot)
    {
        SpawnLobbyActorForPlayer(UpdatedPlayerState, NewAvailableSlot);
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

void AGS_CustomLobbyGM::SpawnLobbyActorForPlayer(AGS_PlayerState* PlayerState, AGS_SpawnSlot* SpawnSlot)
{
    if (!PlayerState || !SpawnSlot || !PawnMappingData || !GetWorld()) return;

    const FAssetToSpawn* SpawnInfo = nullptr;
    if (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
    {
        SpawnInfo = PawnMappingData->GuardianPawnClasses.Find(PlayerState->CurrentGuardianJob);
    }
    else
    {
        SpawnInfo = PawnMappingData->SeekerPawnClasses.Find(PlayerState->CurrentSeekerJob);
    }

    if (SpawnInfo && SpawnInfo->DisplayActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        FTransform SpawnTransform = SpawnSlot->GetActorTransform();
        if (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
        {
            SpawnTransform.SetScale3D(FVector(3.f));
        }

        AGS_LobbyDisplayActor* NewDisplayActor = GetWorld()->SpawnActor<AGS_LobbyDisplayActor>(
            SpawnInfo->DisplayActorClass, SpawnTransform, SpawnParams);

        if (NewDisplayActor)
        {
            NewDisplayActor->CurrentSkeletalMesh = SpawnInfo->SkeletalMeshClass;
            NewDisplayActor->CurrentAnimClass = SpawnInfo->Lobby_AnimBlueprintClass;
            NewDisplayActor->CurrentWeaponMeshList = SpawnInfo->WeaponMeshList;
            NewDisplayActor->CurrentSubMeshList = SpawnInfo->SubSkeletalMeshList;
            NewDisplayActor->bIsReady = PlayerState->bIsReady;
            NewDisplayActor->AssociatedPlayerState = PlayerState;
            NewDisplayActor->OnRep_SetupDisplay();
            NewDisplayActor->OnRep_ReadyState();
            NewDisplayActor->OnRep_PlayerState();

            SpawnedLobbyActors.Add(PlayerState, NewDisplayActor);
        }
    }
}

void AGS_CustomLobbyGM::DestroyLobbyActorForPlayer(AGS_PlayerState* PlayerState)
{
    if (TObjectPtr<AGS_LobbyDisplayActor>* FoundActor = SpawnedLobbyActors.Find(PlayerState))
    {
        if (*FoundActor)
        {
            (*FoundActor)->Destroy();
        }
        SpawnedLobbyActors.Remove(PlayerState);
    }
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
