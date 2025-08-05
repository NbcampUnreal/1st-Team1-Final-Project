// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GS_EnumUtils.h"
#include "Character/Component/GS_StatRow.h"
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
	Occupied	UMETA(DisplayName = "Occupied")
};

UENUM(BlueprintType)
enum class EPlacementResult : uint8
{
	Valid           UMETA(DisplayName = "Valid"),
	ReplaceExisting UMETA(DisplayName = "ReplaceExisting"),
	OutOfBounds     UMETA(DisplayName = "OutOfBounds")
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
struct FArcaneBoardStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGS_StatRow RuneStats;

	UPROPERTY(BlueprintReadWrite)
	FGS_StatRow BonusStats;

	FArcaneBoardStats()
	{
		RuneStats = FGS_StatRow();
		BonusStats = FGS_StatRow();
	}
};

USTRUCT(Atomic, BlueprintType)
struct FPlacedRuneInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 RuneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Pos;

	FPlacedRuneInfo()
	{
		RuneID = 0;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsConnected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 PlacedRuneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* RuneTextureFrag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ConnectedRuneTextureFrag;

	FGridCellData()
	{
		Pos = { 0, 0 };
		State = EGridCellState::Empty;
		bIsSpecialCell = false;
		bIsConnected = false;
		PlacedRuneID = 0;
		RuneTextureFrag = nullptr;
		ConnectedRuneTextureFrag = nullptr;
	}

	FGridCellData(const FIntPoint& InPos, EGridCellState InState, bool InIsSpecialCell = false, bool InIsConnected = false)
	{
		Pos = InPos;
		State = InState;
		bIsSpecialCell = InIsSpecialCell;
		bIsConnected = InIsConnected;
		PlacedRuneID = 0;
		RuneTextureFrag = nullptr;
		ConnectedRuneTextureFrag = nullptr;
	}
};

USTRUCT(Atomic, BlueprintType)
struct FRunePlacementData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPlacedRuneInfo> PlacedRunes;

	FRunePlacementData() {}
};