#pragma once

#include "CoreMinimal.h"
#include "GS_DungeonEditorTypes.h"
#include "Props/Trap/GS_TrapData.h"
#include "GS_PlaceableObjectsRow.generated.h"

class AGS_PlacerBase;

USTRUCT(BlueprintType)
struct FGS_PlaceableObjectsRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PlaceableObjectClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AGS_PlacerBase> ObjectPlacerClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EObjectType ObjectType = EObjectType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="RoomType", EditCondition="ObjectType==EObjectType::Room", EditConditionHides))
	ERoomType RoomType = ERoomType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="TrapType", EditCondition="ObjectType==EObjectType::Trap", EditConditionHides))
	ETrapPlacement TrapType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="MonsterType", EditCondition="ObjectType==EObjectType::Monster", EditConditionHides))
	EMonsterType MonsterType = EMonsterType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="DoorAndWallType", EditCondition="ObjectType==EObjectType::DoorAndWall", EditConditionHides))
	EDoorAndWallType DoorAndWallType = EDoorAndWallType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint ObjectSize = FIntPoint(1, 1);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="RoomCellInfo", EditCondition="ObjectType==EObjectType::Room", EditConditionHides))
	TMap<FIntPoint, EDEditorCellType> RoomCellInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector OffSet = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConstructionCost = 0.0f;
};
