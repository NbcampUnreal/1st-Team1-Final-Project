#include "DungeonEditor/Data/GS_DungeonEditorSaveGame.h"

void UGS_DungeonEditorSaveGame::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// SavedDungeonActorData 등 기존에 저장하던 데이터는 그대로 직렬화
	Ar << SavedDungeonActorData;
	
	// bExcludeDungeonEditingArrays 플래그가 false일 때만 새로운 배열들을 직렬화
	if (!bExcludeDungeonEditingArrays)
	{
		Ar << FloorOccupancyData;
		Ar << CeilingOccupancyData;
	}
}
