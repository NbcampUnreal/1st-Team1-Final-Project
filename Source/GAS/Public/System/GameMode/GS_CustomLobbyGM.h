#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_CustomLobbyGM.generated.h"

UCLASS()
class GAS_API AGS_CustomLobbyGM : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGS_CustomLobbyGM();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	void UpdatePlayerReadyStatus(APlayerState* Player, bool bIsReady);

protected:
	void CheckAllPlayersReady();

	UPROPERTY()
	TMap<TObjectPtr<APlayerState>, bool> PlayerReadyStates;

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	FName NextLevelName = "LoadingLevel";

	UPROPERTY(EditDefaultsOnly, Category = "Game Settings")
	int32 MinPlayersToStart = 2;

};
