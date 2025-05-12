// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GS_ArcaneBoardTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Ares	UMETA(DisplayName = "Ares"),
	Merci	UMETA(DisplayName = "Merci"),
	Chan	UMETA(DisplayName = "Chan")
};

UENUM(BlueprintType)
enum class EGridCellState : uint8
{
	Empty		UMETA(DisplayName = "Empty"),
	Occupied	UMETA(DisplayName = "Occupied"),
	Locked		UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class EPreviewState : uint8
{
	None		UMETA(DisplayName = "None"),
	Valid		UMETA(DisplayName = "Valid"),
	Invalid		UMETA(DisplayName = "Invalid")
};


USTRUCT(Atomic, BlueprintType)
struct FStatEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	FStatEffect()
	{
		StatName = NAME_None;
		Value = 0.0f;
	}

	FStatEffect(FName InStatName, float InValue)
	{
		StatName = InStatName;
		Value = InValue;
	}
};

USTRUCT(Atomic, BlueprintType)
struct FPlacedRuneInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RuneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Pos;

	FPlacedRuneInfo()
	{
		RuneID = -1;
		Pos = { 0, 0 };
	}

	FPlacedRuneInfo(int32 InRuneID, const FIntPoint& InPos)
	{
		RuneID = InRuneID;
		Pos = InPos;
	}
};

USTRUCT(Atomic, BlueprintType)
struct FGridCellData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Pos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGridCellState State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSpecialCell;

	FGridCellData()
	{
		Pos = { 0, 0 };
		State = EGridCellState::Empty;
		bIsSpecialCell = false;
	}

	FGridCellData(const FIntPoint& InPos, EGridCellState InState, bool InIsSpecialCell = false)
	{
		Pos = InPos;
		State = InState;
		bIsSpecialCell = InIsSpecialCell;
	}
};