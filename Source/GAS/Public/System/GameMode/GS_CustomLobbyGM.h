#pragma once

#include "CoreMinimal.h"
#include "System/GS_BaseGM.h"
#include "GS_CustomLobbyGM.generated.h"

class AGS_SpawnSlot;
class AGS_PlayerState;
class APawn;
class UGS_PawnMappingDataAsset;
class AGS_LobbyDisplayActor;

UCLASS()
class GAS_API AGS_CustomLobbyGM : public AGS_BaseGM
{
	GENERATED_BODY()
	
public:
	AGS_CustomLobbyGM();
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	void UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady);

protected:
	void CheckAllPlayersReady();

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, bool> PlayerReadyStates;

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	FName NextLevelName = "InGameTestLevel_v2"; // 테스트 후에 다시 LoadingLevel로 바꿔놔야함!!!!!!!!!!!!!!!!!!!

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	int32 MinPlayersToStart = 2;

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
	TMap<TObjectPtr<AGS_PlayerState>, TObjectPtr<AGS_LobbyDisplayActor>> SpawnedLobbyActors;
	UPROPERTY()
	TMap<TObjectPtr<AGS_PlayerState>, TObjectPtr<AGS_SpawnSlot>> PlayerToSlotMap;

	void CollectSpawnSlots();
	void SpawnLobbyActorForPlayer(AGS_PlayerState* PlayerState, AGS_SpawnSlot* SpawnSlot);
	void DestroyLobbyActorForPlayer(AGS_PlayerState* PlayerState);
	AGS_SpawnSlot* FindAvailableSlotForPlayer(AGS_PlayerState* PlayerState);
};
