#pragma once

#include "CoreMinimal.h"
#include "System/GS_BaseGM.h"
#include "System/GS_PlayerRole.h"
#include "GS_ResultGM.generated.h"

class UGS_PawnMappingDataAsset;
class AGS_SpawnSlot;
class AGS_PlayerState;

UCLASS()
class GAS_API AGS_ResultGM : public AGS_BaseGM
{
	GENERATED_BODY()
	
public:
	AGS_ResultGM();

	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void StartPlay() override;

private:
	void ExtractResult();
	EGameResult CurrentResult;
public:
	EGameResult GetGameResult() const { return CurrentResult; }

	void SetCameraTransform(AController* NewPlayer);

	void CollectSpawnSlots();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Configuration")
	UGS_PawnMappingDataAsset* PawnMappingData;

	TArray<AGS_SpawnSlot*> GuardianSlots;
	TArray<AGS_SpawnSlot*> SeekerSlots;

	UPROPERTY()
	TMap<TObjectPtr<AGS_PlayerState>, TObjectPtr<APawn>> SpawnedLobbyPawns;

	void SpawnPlayersWithPoseByResult(EGameResult CurrentResult);

	void SpawnLobbyPawnForPlayer(AGS_PlayerState* PlayerState, AGS_SpawnSlot* SpawnSlot, const FActorSpawnParameters& SpawnParams);
	
};
