#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_BaseGM.generated.h"

UCLASS()
class GAS_API AGS_BaseGM : public AGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGameLogic(); 
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	FString CurrentSaveSlotName = TEXT("Preset_0");

	// --- [내비메시 관련 코드 추가] ---
	void CheckNavMeshBuildStatus();
	void OnNavMeshBuildComplete();

	FTimerHandle NavMeshBuildTimerHandle;
};