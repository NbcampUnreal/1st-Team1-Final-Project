#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_BoseLevelGM.generated.h"

UCLASS()
class GAS_API AGS_BoseLevelGM : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGS_BoseLevelGM();
	virtual TSubclassOf<APlayerController> GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual void StartPlay() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void Logout(AController* Exiting) override;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> SeekerControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Controller Classes")
	TSubclassOf<APlayerController> GuardianControllerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Pawn Classes")
	TSubclassOf<APawn> SeekerPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Player Pawn Classes")
	TSubclassOf<APawn> GuardianPawnClass;
	
};
