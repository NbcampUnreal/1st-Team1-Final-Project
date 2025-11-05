#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_CustomLobbyGM.generated.h"

class AGS_SpawnSlot;
class AGS_PlayerState;
class APawn;
class UGS_PawnMappingDataAsset;
class AGS_LobbyDisplayActor;

UCLASS()
class GAS_API AGS_CustomLobbyGM : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGS_CustomLobbyGM();
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	void HandlePlayerReadyInLobby(APlayerController* PlayerController);
	virtual void Logout(AController* Exiting) override;
	void UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady);

	/** 클라이언트의 요청을 받아 GameSessionId를 찾아 보내줍니다. */
	void RequestGameSessionIdForClient(APlayerController* RequestingController);

protected:
	void CheckAllPlayersReady();

	void DoServerTravel();

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, bool> PlayerReadyStates;

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	FName NextLevelName = "InGameLevel";
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	int32 MinPlayersToStart = 1;

private:
	UPROPERTY()
	TMap<FString, FString> PendingPlayerSessions;

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
	TMap<AGS_PlayerState*, TObjectPtr<AGS_LobbyDisplayActor>> SpawnedLobbyActors;
	UPROPERTY()
	TMap<AGS_PlayerState*, TObjectPtr<AGS_SpawnSlot>> PlayerToSlotMap;

	void CollectSpawnSlots();
	void SpawnLobbyActorForPlayer(AGS_PlayerState* PlayerState, AGS_SpawnSlot* SpawnSlot);
	void DestroyLobbyActorForPlayer(AGS_PlayerState* PlayerState);
	AGS_SpawnSlot* FindAvailableSlotForPlayer(AGS_PlayerState* PlayerState);

};
