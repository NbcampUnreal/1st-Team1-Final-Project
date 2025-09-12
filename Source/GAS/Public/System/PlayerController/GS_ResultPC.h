#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_ResultPC.generated.h"

class UGS_GameResultUI;

UCLASS()
class GAS_API AGS_ResultPC : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGS_GameResultUI> ResultUIClass;

public:
	UFUNCTION(Client, Reliable)
	void Client_ShowResultUI();
	
	UFUNCTION(Server, Reliable)
	void Server_RequestTravelToLobby();
};
