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
	void ClearData() { SavedDungeonActorData.Empty(); FloorOccupancyData.Empty(); CeilingOccupancyData.Empty(); }
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

	// 직렬화 제외 플래그
	UPROPERTY(Transient)
	bool bExcludeDungeonEditingArrays = false;

	// 직렬화 함수 오버라이드
	virtual void Serialize(FArchive& Ar) override;
	
protected:
	UPROPERTY()
	TArray<FDESaveData> SavedDungeonActorData;
};
