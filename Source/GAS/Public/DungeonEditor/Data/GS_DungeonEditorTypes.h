#pragma once

#include "CoreMinimal.h"
#include "GS_DungeonEditorTypes.generated.h"

enum class ETrapPlacement : uint8;

UENUM(BlueprintType)
enum class EDEditorCellType : uint8
{
	None	UMETA(DisplayName = "None"),
	VerticalPlaceable	UMETA(DisplayName = "VerticalPlaceable"),
	HorizontalPlaceable	UMETA(DisplayName = "HorizontalPlaceable"),
	WallAndDoorPlaceable	UMETA(DisplayName = "WallAndDoorPlaceable"),
	WallPlace UMETA(DisplayName = "WallPlace"),
	FloorPlace UMETA(DisplayName = "FloorPlace"),
	CeilingPlace UMETA(DisplayName = "CeilingPlace"),
	Wall UMETA(DisplayName = "Wall"),
	Door UMETA(DisplayName = "Door")
};

UENUM(BlueprintType)
enum class EObjectType : uint8
{
	Room	UMETA(DisplayName = "Room"),
	Trap	UMETA(DisplayName = "Trap"),
	Monster UMETA(DisplayName = "Monster"),
	DoorAndWall UMETA(DisplayName = "DoorAndWall"),
	None	UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	Normal	UMETA(DisplayName = "Normal"),
	Elite	UMETA(DisplayName = "Elite"),
	None	UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Default	UMETA(DisplayName = "Default"),
	BossRoom UMETA(DisplayName = "BossRoom"),
	Template UMETA(DisplayName = "Template"),
	None	UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EDoorAndWallType : uint8
{
	Wall	UMETA(DisplayName = "Wall"),
	Door UMETA(DisplayName = "Door"),
	None	UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EPlacerDirectionType : uint8
{
	Forward		UMETA(DisplayName = "Forward"),
	Right		UMETA(DisplayName = "Right"),
	Backward	UMETA(DisplayName = "Backward"),
	Left		UMETA(DisplayName = "Left")
};

USTRUCT(Atomic, BlueprintType)
struct FDEOccupancyData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	EDEditorCellType FloorOccupancyData;
	UPROPERTY()
	EDEditorCellType CeilingOccupancyData;

	UPROPERTY()
	TObjectPtr<AActor> RoomOccupancyActor;
	UPROPERTY()
	TObjectPtr<AActor> FloorOccupancyActor;
	UPROPERTY()
	TObjectPtr<AActor> CeilingOccupancyActor;
	UPROPERTY()
	TObjectPtr<AActor> WallAndDoorOccupancyActor;
};

USTRUCT(Atomic, BlueprintType)
struct FDESaveData
{
	GENERATED_BODY()
	
	// UPROPERTY()
	// TSubclassOf<AActor> SpawnActorClass;
	UPROPERTY()
	FString SpawnActorClassPath;
	UPROPERTY()
	FTransform SpawnTransform;
	UPROPERTY()
	TArray<FIntPoint> CellCoord;
	UPROPERTY()
	EObjectType ObjectType;
	UPROPERTY()
	ETrapPlacement TrapPlacement;
	UPROPERTY()
	float ConstructionCost;
	FDESaveData() {}
};

FORCEINLINE FArchive& operator<<(FArchive& Ar, FDESaveData& Data)
{
	// 구조체의 각 멤버 변수를 순서대로 직렬화합니다.
	// 저장할 때와 불러올 때의 순서가 반드시 같아야 합니다.
	// Ar << Data.SpawnActorClass;
	Ar << Data.SpawnActorClassPath;
	Ar << Data.SpawnTransform;
	Ar << Data.CellCoord;
	Ar << Data.ObjectType;
	Ar << Data.TrapPlacement;
	Ar << Data.ConstructionCost;
	return Ar;
}