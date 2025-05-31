#include "System/GameMode/GS_BossLevelGM.h"
#include "System/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"

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

	return (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? *SeekerPawnClass : *GuardianPawnClass;
}

void AGS_BossLevelGM::StartPlay()
{
	Super::StartPlay();
	FString ResultLevelName = TEXT("ResultLevel");
	UGameplayStatics::LoadStreamLevel(this, FName(*ResultLevelName), false, false, FLatentActionInfo());
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
				UE_LOG(LogTemp, Log, TEXT("AGS_BossLevelGM: Player %s is Seeker. Looking for PlayerStartTag: %s"), *Player->GetName(), *PlayerStartTagToFind);
			}
			else if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
			{
				PlayerStartTagToFind = TEXT("GuardianStart");
				UE_LOG(LogTemp, Log, TEXT("AGS_BossLevelGM: Player %s is Guardian. Looking for PlayerStartTag: %s"), *Player->GetName(), *PlayerStartTagToFind);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_BossLevelGM::ChoosePlayerStart: Player %s does not have AGS_PlayerState. Using default PlayerStart."), *Player->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_BossLevelGM::ChoosePlayerStart: PlayerController is NULL. Using default PlayerStart."));
	}

	AActor* FoundPlayerStart = FindPlayerStart_Implementation(Player, PlayerStartTagToFind);

	if (FoundPlayerStart)
	{
		UE_LOG(LogTemp, Log, TEXT("AGS_BossLevelGM: Selected PlayerStart: %s for Player: %s with Tag: %s"),
			*FoundPlayerStart->GetName(), Player ? *Player->GetName() : TEXT("UnknownPlayer"), *PlayerStartTagToFind);
		return FoundPlayerStart;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_BossLevelGM: Could not find a PlayerStart with tag '%s' for Player: %s. Falling back to Super::ChoosePlayerStart_Implementation."),
			*PlayerStartTagToFind, Player ? *Player->GetName() : TEXT("UnknownPlayer"));
		return Super::ChoosePlayerStart_Implementation(Player);
	}
}

void AGS_BossLevelGM::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
