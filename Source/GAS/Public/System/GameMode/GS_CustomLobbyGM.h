#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_CustomLobbyGM.generated.h"

class AGS_SpawnSlot;
class AGS_PlayerState;
class APawn;
class UGS_PawnMappingDataAsset;

UCLASS()
class GAS_API AGS_CustomLobbyGM : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGS_CustomLobbyGM();
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	void UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady);

protected:
	void CheckAllPlayersReady();

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, bool> PlayerReadyStates;

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	FName NextLevelName = "InGameTestLevel"; // 테스트 후에 다시 LoadingLevel로 바꿔놔야함!!!!!!!!!!!!!!!!!!!

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	int32 MinPlayersToStart = 1;

	//폰 배치
public:
	void HandlePlayerStateUpdated(AGS_PlayerState* UpdatedPlayerState);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TObjectPtr<UGS_PawnMappingDataAsset> PawnMappingData;

private:
	TArray<AGS_SpawnSlot*> GuardianSlots;
	TArray<AGS_SpawnSlot*> SeekerSlots;

	UPROPERTY()
	TMap<TObjectPtr<AGS_PlayerState>, TObjectPtr<APawn>> SpawnedLobbyPawns;
	/*UPROPERTY()
	TMap<TObjectPtr<AGS_PlayerState>, TObjectPtr<AActor>> SpawnedLobbyActors;*/

	void CollectSpawnSlots();
	void UpdateLobbyPawns();
};
