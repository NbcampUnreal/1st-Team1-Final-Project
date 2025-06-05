#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "System/GS_PlayerRole.h"
#include "GS_ResultGM.generated.h"

UCLASS()
class GAS_API AGS_ResultGM : public AGameMode
{
	GENERATED_BODY()
	
public:
	AGS_ResultGM();

	/*void ExtractResult();

	void SpawnPlayersWithPoseByResult(EGameResult Result);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Configuration")
	UGS_PawnMappingDataAsset* PawnMappingDataAsset;*/
};
