#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_InGameGM.generated.h"



UCLASS()
class GAS_API AGS_InGameGM : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGS_InGameGM();
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void PostSeamlessTravel() override;
	virtual void BeginPlay() override;

	void SpawnAllPlayersSafely();
	bool bSpawnDelegateBound = false;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> SeekerControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> GuardianControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> RTSControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Pawn Classes")
	TSubclassOf<APawn> SeekerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Pawn Classes")
	TSubclassOf<APawn> GuardianPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Pawn Classes")
	TSubclassOf<APawn> RTSPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD Classes")
	TSubclassOf<AHUD> SeekerHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD Classes")
	TSubclassOf<AHUD> RTSHUDClass;

	//void HandleControllerChangeTriggerEvent();
	
};
