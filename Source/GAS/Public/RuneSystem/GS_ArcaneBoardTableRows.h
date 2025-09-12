// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardTableRows.generated.h"

class UGS_GridLayoutDataAsset;

USTRUCT(BlueprintType)
struct FRuneTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 RuneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RuneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FIntPoint, UTexture2D*> RuneShape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FIntPoint, UTexture2D*> ConnectedRuneShape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint RuneSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RuneTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStatEffect StatEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlaced;
};

USTRUCT(BlueprintType)
struct FGridLayoutTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UGS_GridLayoutDataAsset> GridLayoutAsset;
};