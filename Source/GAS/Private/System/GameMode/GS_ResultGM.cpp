#include "System/GameMode/GS_ResultGM.h"
#include "System/GameState/GS_ResultGS.h"
#include "System/PlayerController/GS_ResultPC.h"
#include "System/GS_PlayerState.h"
#include "System/GS_SpawnSlot.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

AGS_ResultGM::AGS_ResultGM()
{
	GameStateClass = AGS_ResultGS::StaticClass();
	PlayerStateClass = AGS_PlayerState::StaticClass();
	bUseSeamlessTravel = true;
}

void AGS_ResultGM::RestartPlayer(AController* NewPlayer)
{
	//Super::RestartPlayer(NewPlayer);

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (APlayerController* PC = Cast<APlayerController>(PS->GetOwner()))
		{
			if (AGS_ResultPC* GS_PC = Cast<AGS_ResultPC>(PC))
			{
				GS_PC->Client_ShowResultUI();
			}
		}
	}
    SetCameraTransform(NewPlayer);
}

void AGS_ResultGM::StartPlay()
{
	Super::StartPlay();

    CollectSpawnSlots();

	ExtractResult();
	if (CurrentResult == EGameResult::GR_SeekersLost || CurrentResult == EGameResult::GR_SeekersWon)
	{
		SpawnPlayersWithPoseByResult(CurrentResult);
	}
}

void AGS_ResultGM::ExtractResult()
{
	if (!GameState) return;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
		if (GS_PS)
		{
            CurrentResult = GS_PS->CurrentGameResult;
			return;
		}
	}
}

void AGS_ResultGM::SetCameraTransform(AController* NewPlayer)
{
    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("DefaultSpectatorCamera"), FoundCameras);

    AActor* DefaultCamera = nullptr;
    if (FoundCameras.Num() > 0)
    {
        DefaultCamera = FoundCameras[0];
    }

    if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
    {
        PC->SetViewTargetWithBlend(DefaultCamera, 0.0f, EViewTargetBlendFunction::VTBlend_Linear, 0.0f, true);
    }
}

void AGS_ResultGM::CollectSpawnSlots()
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

void AGS_ResultGM::SpawnPlayersWithPoseByResult(EGameResult Result)
{
	if (Result != EGameResult::GR_SeekersLost && Result != EGameResult::GR_SeekersWon)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_ResultGM::SpawnPlayersWithPoseByResult - Invalid Result: %s"), *UEnum::GetValueAsString(Result));
		return;
	}

	if (!GameState) return;
    if (!PawnMappingData) return;
    for (auto& Pair : SpawnedLobbyPawns)
    {
        if (Pair.Value)
        {
            Pair.Value->Destroy();
        }
    }
    SpawnedLobbyPawns.Empty();
    
    TArray<AGS_PlayerState*> Guardians;
    TArray<AGS_PlayerState*> Seekers;

    for (APlayerState* PS : GameState->PlayerArray)
    {
        AGS_PlayerState* Player = Cast<AGS_PlayerState>(PS);
        if (Player)
        {
            if (Player->CurrentPlayerRole == EPlayerRole::PR_Guardian) Guardians.Add(Player);
            else if (Player->CurrentPlayerRole == EPlayerRole::PR_Seeker) Seekers.Add(Player);
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (Guardians.Num() == 1)
    {
        if (GuardianSlots.Num() >= 2)
        {
            const bool bDidGuardiansWin = (Result == EGameResult::GR_SeekersLost);
            AGS_SpawnSlot* GuardianSlot = bDidGuardiansWin ? GuardianSlots[0] : GuardianSlots[1];

            SpawnLobbyPawnForPlayer(Guardians[0], GuardianSlot, SpawnParams);
        }
    }

    for (int32 i = 0; i < Seekers.Num(); ++i)
    {
        if (SeekerSlots.IsValidIndex(i))
        {
            SpawnLobbyPawnForPlayer(Seekers[i], SeekerSlots[i], SpawnParams);
        }
        else
        {
            break;
        }
    }
}

void AGS_ResultGM::SpawnLobbyPawnForPlayer(AGS_PlayerState* PlayerState, AGS_SpawnSlot* SpawnSlot, const FActorSpawnParameters& SpawnParams)
{
    if (!PlayerState || !SpawnSlot || !PawnMappingData || !GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnLobbyPawnForPlayer: Invalid input provided. PlayerState: %d, SpawnSlot: %d"), IsValid(PlayerState), IsValid(SpawnSlot));
        return;
    }

    const FAssetToSpawn* SpawnInfo = nullptr;

    if (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
    {
        SpawnInfo = PawnMappingData->GuardianPawnClasses.Find(PlayerState->CurrentGuardianJob);
    }
    else if (PlayerState->CurrentPlayerRole == EPlayerRole::PR_Seeker)
    {
        SpawnInfo = PawnMappingData->SeekerPawnClasses.Find(PlayerState->CurrentSeekerJob);
    }

    if (SpawnInfo && SpawnInfo->PawnClass)
    {
        FTransform SpawnTransform = SpawnSlot->GetActorTransform();
        SpawnTransform.AddToTranslation(FVector(0, 0, 100.0f));
        
        APawn* NewPawn = GetWorld()->SpawnActor<APawn>(SpawnInfo->PawnClass, SpawnTransform, SpawnParams);
        if (NewPawn)
        {
            SpawnedLobbyPawns.Add(PlayerState, NewPawn);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not find SpawnInfo for Player %s (Role: %s)"), *PlayerState->GetPlayerName(), *UEnum::GetValueAsString(PlayerState->CurrentPlayerRole));
    }
}