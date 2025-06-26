#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DungeonEditor/Data/GS_DungeonEditorTypes.h"
#include "GS_DungeonEditorSaveGame.generated.h"

struct FDESaveData;

UCLASS()
class GAS_API UGS_DungeonEditorSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void ClearData() { SavedDungeonActorData.Empty(); }
	UFUNCTION()
	void AddSaveData(const FDESaveData& InSaveData) { SavedDungeonActorData.Add(InSaveData); }
	UFUNCTION()
	int GetDataCount() const { return SavedDungeonActorData.Num(); }
	UFUNCTION()
	TArray<FDESaveData>& GetSaveDatas() { return SavedDungeonActorData; }

	UPROPERTY()
	TMap<FIntPoint,EDEditorCellType> FloorOccupancyData;
	UPROPERTY()
	TMap<FIntPoint,EDEditorCellType> CeilingOccupancyData;
	
protected:
	UPROPERTY()
	TArray<FDESaveData> SavedDungeonActorData;
};
