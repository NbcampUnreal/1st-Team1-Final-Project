#pragma once

#include "CoreMinimal.h"
#include "System/GS_BaseGM.h"
#include "System/GS_PlayerRole.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "Props/Trap/GS_TrapManager.h"
#include "GS_InGameGM.generated.h"

class AGS_PlayerState;

UCLASS()
class GAS_API AGS_InGameGM : public AGS_BaseGM
{
	GENERATED_BODY()
	
public:
	AGS_InGameGM();
	virtual TSubclassOf<APlayerController> GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void StartPlay() override;
	virtual void BeginPlay() override;

	virtual void Logout(AController* Exiting) override;

protected:
	void StartMatchCheck();

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

	//Trap
	UPROPERTY()
	AGS_TrapManager* TrapManager;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	TSubclassOf<class AGS_TrapManager>TrapManagerClass;
	
	AGS_TrapManager* GetTrapManager() const;

public:
	void HandlePlayerAliveStatusChanged(AGS_PlayerState* PlayerState, bool bIsAlive);

	void CheckAllPlayersDead();

	void BindToPlayerState(APlayerController* PlayerController);

	void OnTimerEnd();

	UFUNCTION(BlueprintCallable)
	void EndGame(EGameResult Result);

private:
	void SetGameResultOnAllPlayers(EGameResult Result);

	FTimerHandle MatchStartTimerHandle;

	bool bMatchHasStarted;
	bool bGameEnded;
};
