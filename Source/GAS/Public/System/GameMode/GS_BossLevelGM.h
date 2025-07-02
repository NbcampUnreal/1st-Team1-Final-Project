#pragma once

#include "CoreMinimal.h"
#include "System/GS_BaseGM.h"
#include "System/GS_PlayerRole.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "GS_BossLevelGM.generated.h"

class AGS_PlayerState;

UCLASS()
class GAS_API AGS_BossLevelGM : public AGS_BaseGM
{
	GENERATED_BODY()
	
public:
	AGS_BossLevelGM();
	virtual TSubclassOf<APlayerController> GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual void StartPlay() override;
	virtual void StartMatchWhenAllReady() override;
	void BindToPlayerState(APlayerController* PlayerController);
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void Logout(AController* Exiting) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void OnTimerEnd();

	void EndGame(EGameResult Result);

	void SetGameResultOnAllPlayers(EGameResult Result);

	void HandlePlayerAliveStatusChanged(AGS_PlayerState* PlayerState, bool bIsAlive);

	void CheckAllPlayersDead();

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> SeekerControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> GuardianControllerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Configuration")
	UGS_PawnMappingDataAsset* PawnMappingDataAsset;
	
	FTimerHandle MatchStartTimerHandle;
};
