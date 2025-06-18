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
		UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s logged in. Total players: %d"), *PS->GetPlayerName(), PlayerReadyStates.Num());
	}

    UpdateLobbyPawns();
}

void AGS_CustomLobbyGM::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);

    if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
    {
        AGS_PlayerState* PS = PC->GetPlayerState<AGS_PlayerState>();
        if (PS)
        {
            PlayerReadyStates.Add(PS, false);
            PS->InitializeDefaults();
            UE_LOG(LogTemp, Log, TEXT("LobbyGM: Player %s has (re)entered the lobby. Resetting state."), *PS->GetPlayerName());
        }
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

    if (PS && SpawnedLobbyActors.Contains(PS))
    {
        AActor* PawnToDestroy = SpawnedLobbyActors[PS];
        if (PawnToDestroy)
        {
            PawnToDestroy->Destroy();
        }
        SpawnedLobbyActors.Remove(PS);
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
            AGS_PlayerState* GSPlayer = Cast<AGS_PlayerState>(Player);
            if (SpawnedLobbyActors.Contains(GSPlayer))
            {
                if (AGS_LobbyDisplayActor* DisplayActor = Cast<AGS_LobbyDisplayActor>(SpawnedLobbyActors[GSPlayer]))
                {
                    DisplayActor->SetReadyState(bIsReady);
                }
            }
			CheckAllPlayersReady();
		}
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
    if (!PawnMappingData || !GS)
    {
        return;
    }

    for (auto& Pair : SpawnedLobbyActors)
    {
        if (Pair.Value)
        {
            Pair.Value->Destroy();
        }
    }
    SpawnedLobbyActors.Empty();

    TArray<AGS_PlayerState*> Guardians;
    TArray<AGS_PlayerState*> Seekers;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AGS_PlayerState* Player = Cast<AGS_PlayerState>(PS);
        if (Player)
        {
            UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyGM::UpdateLobbyPawns - Player: %s, Role: %s"), *Player->GetPlayerName(), *UEnum::GetValueAsString(Player->CurrentPlayerRole));
            if (Player->CurrentPlayerRole == EPlayerRole::PR_Guardian) Guardians.Add(Player);
            else if (Player->CurrentPlayerRole == EPlayerRole::PR_Seeker) Seekers.Add(Player);
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < Guardians.Num() && i < GuardianSlots.Num(); ++i)
    {
        AGS_PlayerState* GuardianPS = Guardians[i];
        AGS_SpawnSlot* Slot = GuardianSlots[i];

        const FAssetToSpawn* SpawnInfo = PawnMappingData->GuardianPawnClasses.Find(GuardianPS->CurrentGuardianJob);
        if (SpawnInfo && SpawnInfo->SkeletalMeshClass)
        {
            FTransform SpawnTransform = Slot->GetActorTransform();
            SpawnTransform.SetScale3D(FVector(3.0f));
            AGS_LobbyDisplayActor* NewDisplayActor = GetWorld()->SpawnActor<AGS_LobbyDisplayActor>(SpawnInfo->DisplayActorClass, SpawnTransform, SpawnParams);
            if (NewDisplayActor)
            {
                NewDisplayActor->SetupDisplay(SpawnInfo->SkeletalMeshClass, SpawnInfo->Lobby_AnimBlueprintClass, SpawnInfo->WeaponMeshList);
                NewDisplayActor->SetReadyState(GuardianPS->bIsReady);

                SpawnedLobbyActors.Add(GuardianPS, NewDisplayActor);
            }
        }
    }

    for (int32 i = 0; i < Seekers.Num() && i < SeekerSlots.Num(); ++i)
    {
        AGS_PlayerState* SeekerPS = Seekers[i];
        AGS_SpawnSlot* Slot = SeekerSlots[i];

        const FAssetToSpawn* SpawnInfo = PawnMappingData->SeekerPawnClasses.Find(SeekerPS->CurrentSeekerJob);
        if (SpawnInfo && SpawnInfo->SkeletalMeshClass)
        {
            FTransform SpawnTransform = Slot->GetActorTransform();
            AGS_LobbyDisplayActor* NewDisplayActor = GetWorld()->SpawnActor<AGS_LobbyDisplayActor>(SpawnInfo->DisplayActorClass, SpawnTransform, SpawnParams);
            if (NewDisplayActor)
            {
                NewDisplayActor->SetupDisplay(SpawnInfo->SkeletalMeshClass, SpawnInfo->Lobby_AnimBlueprintClass, SpawnInfo->WeaponMeshList);
                NewDisplayActor->SetReadyState(SeekerPS->bIsReady);
                SpawnedLobbyActors.Add(SeekerPS, NewDisplayActor);
            }
        }
    }
}
