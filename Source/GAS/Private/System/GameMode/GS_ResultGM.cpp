#include "System/GameMode/GS_ResultGM.h"
#include "System/GameState/GS_ResultGS.h"
#include "System/PlayerController/GS_ResultPC.h"
#include "System/GS_PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"


AGS_ResultGM::AGS_ResultGM()
{
	GameStateClass = AGS_ResultGS::StaticClass();
	PlayerStateClass = AGS_PlayerState::StaticClass();
	bUseSeamlessTravel = true;
}

void AGS_ResultGM::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

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
}

void AGS_ResultGM::ExtractResult()
{
	if (!GameState) return;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
		if (GS_PS)
		{
			Result = GS_PS->CurrentGameResult;
			return;
		}
	}
}

//void AGS_ResultGM::SpawnPlayersWithPoseByResult(EGameResult Result)
//{
//	if (Result != EGameResult::GR_SeekersLost && Result != EGameResult::GR_SeekersWon)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("AGS_ResultGM::SpawnPlayersWithPoseByResult - Invalid Result: %s"), *UEnum::GetValueAsString(Result));
//		return;
//	}
//
//	if (!GameState) return;
//	if (!PawnMappingDataAsset)
//	{
//		UE_LOG(LogTemp, Error, TEXT("AGS_BossLevelGM: PawnMappingDataAsset is NOT ASSIGNED in the GameMode Blueprint! Attempting to use super's default."));
//		return;
//	}
//	UClass* ResolvedPawnClass = nullptr;
//	int32 SeekerNum = 0;
//
//	for (APlayerState* PS : GameState->PlayerArray)
//	{
//		AGS_PlayerState* GS_PS = Cast<AGS_PlayerState>(PS);
//		if (!GS_PS)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("AGS_ResultGM::SpawnPlayersWithPoseByResult - Failed to Cast."));
//			continue;
//		}
//			
//
//		if (GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
//		{
//			const TSubclassOf<APawn>* FoundPawnClass = PawnMappingDataAsset->SeekerPawnClasses.Find(GS_PS->CurrentSeekerJob);
//			if (FoundPawnClass && *FoundPawnClass)
//			{
//				ResolvedPawnClass = *FoundPawnClass;
//			}
//			else
//			{
//				ResolvedPawnClass = PawnMappingDataAsset->DefaultGuardianPawn;
//			}
//			SeekerNum++;
//		}
//		else if (GS_PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
//		{
//			const TSubclassOf<APawn>* FoundPawnClass = PawnMappingDataAsset->GuardianPawnClasses.Find(GS_PS->CurrentGuardianJob);
//			if (FoundPawnClass && *FoundPawnClass)
//			{
//				ResolvedPawnClass = *FoundPawnClass;
//			}
//			else
//			{
//				ResolvedPawnClass = PawnMappingDataAsset->DefaultSeekerPawn;
//			}
//		}
//
//
//		if (ResolvedPawnClass)
//		{
//			if (Result == EGameResult::GR_SeekersLost)
//			{
//				if (GS_PS->CurrentPlayerRole != EPlayerRole::PR_Seeker)
//				{
//					FActorSpawnParameters SpawnParams;
//					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//					SpawnParams.Owner = this;
//					FVector SpawnLocation = FVector(0.f, -300.f + 200.f * SeekerNum, 190.f);
//					FRotator SpawnRotation = FRotator::ZeroRotator;
//					GetWorld()->SpawnActor<APawn>(ResolvedPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
//					//TODO: 래그돌 처리
//				}
//				else
//				{
//					FActorSpawnParameters SpawnParams;
//					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//					SpawnParams.Owner = this;
//					FVector SpawnLocation = FVector(-300.f, 0.f, 300.f);
//					FRotator SpawnRotation = FRotator::ZeroRotator;
//					GetWorld()->SpawnActor<APawn>(ResolvedPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
//					
//				}
//			}
//			else if (Result == EGameResult::GR_SeekersWon)
//			{
//				if (GS_PS->CurrentPlayerRole != EPlayerRole::PR_Seeker)
//				{
//					FActorSpawnParameters SpawnParams;
//					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//					SpawnParams.Owner = this;
//					FVector SpawnLocation = FVector(0.f, -300.f + 200.f * SeekerNum, 190.f);
//					FRotator SpawnRotation = FRotator::ZeroRotator;
//					GetWorld()->SpawnActor<APawn>(ResolvedPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
//				}
//				else
//				{
//					FActorSpawnParameters SpawnParams;
//					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//					SpawnParams.Owner = this;
//					FVector SpawnLocation = FVector(300.f, 0.f, 300.f);
//					FRotator SpawnRotation = FRotator::ZeroRotator;
//					GetWorld()->SpawnActor<APawn>(ResolvedPawnClass, SpawnLocation, SpawnRotation, SpawnParams);
//					//TODO: 래그돌 처리
//				}
//			}
//			else
//			{
//				UE_LOG(LogTemp, Warning, TEXT("AGS_ResultGM::SpawnPlayersWithPoseByResult - Invalid Result: %s"), *UEnum::GetValueAsString(Result));
//			}
//		}
//	}
//}
