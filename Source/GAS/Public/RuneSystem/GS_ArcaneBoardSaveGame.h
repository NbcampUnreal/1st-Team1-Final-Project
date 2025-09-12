// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardSaveGame.generated.h"

/**
 * 아케인 보드 시스템의 로컬 플레이어별 저장 데이터
 */
UCLASS()
class GAS_API UGS_ArcaneBoardSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<ECharacterClass, FArcaneBoardPresets> SavedRunesByClass;

	TSet<uint8> OwnedRuneIDs;
};
