#pragma once

#include "CoreMinimal.h"
#include "System/GS_BaseGM.h"
#include "System/GS_PlayerRole.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "Props/Trap/GS_TrapManager.h"
#include "GS_InGameGM.generated.h"

class AGS_RTSCamera;
class AGS_PlayerState;
struct FDESaveData;
class AGS_Monster;

UCLASS()
class GAS_API AGS_InGameGM : public AGS_BaseGM
{
	GENERATED_BODY()
	
public:
	AGS_InGameGM();

protected:
	virtual TSubclassOf<APlayerController> GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	//virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Logout(AController* Exiting) override;
	void DelayedRestartPlayer();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Configuration")
	UGS_PawnMappingDataAsset* PawnMappingDataAsset;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> SeekerControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> GuardianControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> RTSControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Pawn Classes")
	TSubclassOf<APawn> RTSPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD Classes")
	TSubclassOf<AHUD> SeekerHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD Classes")
	TSubclassOf<AHUD> RTSHUDClass;
	
	void HandlePlayerAliveStatusChanged(AGS_PlayerState* PlayerState, bool bIsAlive);

	void CheckAllPlayersDead();

	void BindToPlayerState(APlayerController* PlayerController);

	void OnTimerEnd();

	UFUNCTION(BlueprintCallable)
	void EndGame(EGameResult Result);

private:
	void SetGameResultOnAllPlayers(EGameResult Result);
	
	TArray<AController*> PendingPlayers;

	UPROPERTY()
	TArray<TObjectPtr<AGS_Monster>> SpawnedMonsters;
	//던전 스폰
protected:
	// 동적으로 생성된 던전 액터들을 추적하는 배열
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SpawnedDungeonActors; 
	
	void SpawnDungeonFromArray(const TArray<FDESaveData>& SaveData);

	UPROPERTY()
	TArray<FDESaveData> CachedSaveData;

	// --- [내비메시 관련 코드 추가] ---
	void CheckNavMeshBuildStatus();
	void OnNavMeshBuildComplete();

	FTimerHandle NavMeshBuildTimerHandle;
};
