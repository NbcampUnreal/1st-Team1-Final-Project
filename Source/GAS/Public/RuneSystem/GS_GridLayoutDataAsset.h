// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_GridLayoutDataAsset.generated.h"

/**
 * 직업별 그리드 레이아웃을 저장하는 데이터 에셋
 */
UCLASS(BlueprintType)
class GAS_API UGS_GridLayoutDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Layout")
	ECharacterClass CharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Layout")
	FIntPoint GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Layout")
	TArray<FGridCellData> GridCells;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid Layout")
	TSoftObjectPtr<UTexture2D> BgTexture;
};