#pragma once

#include "CoreMinimal.h"
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
	FIntPoint ObjectSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture> Icon;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TObjectPtr<UStaticMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZOffSet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConstructionCost;
};
