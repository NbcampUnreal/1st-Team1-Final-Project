// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardManager.generated.h"

/**
 * 룬 시스템의 실질적인 로직을 처리하는 매니저
 */
UCLASS()
class GAS_API UGS_ArcaneBoardManager : public UObject
{
	GENERATED_BODY()

public:
	UGS_ArcaneBoardManager();
	
	UPROPERTY()
	ECharacterClass CurrClass;

	UPROPERTY()
	TArray<FPlacedRuneInfo> PlacedRunes;

	UPROPERTY()
	TMap<FName, float> AppliedStatEffects;

	UPROPERTY()
	TMap<FName, float> CurrStatEffects;

	UPROPERTY()
	UDataTable* RuneTable;

	UPROPERTY()
	UDataTable* GridLayoutTable;

	UFUNCTION(BlueprintCallable)
	bool SetCurrClass(ECharacterClass NewClass);

	UFUNCTION(BlueprintCallable)
	bool CanPlaceRuneAt(int32 RuneID, const FIntPoint& Pos);

	UFUNCTION(BlueprintCallable)
	bool RemoveRune(int32 RuneID);

	UFUNCTION(BlueprintCallable)
	TMap<FName, float> CalculateStatEffects();

	UFUNCTION(BlueprintCallable)
	void ApplyChanges();

	UFUNCTION(BlueprintCallable)
	void ResetAllRune();

private:

	UPROPERTY()
	TArray<int32> ConnectedRuneIDs;

	void UpdateConnections();

	void FindConnectedRunes(const TArray<FPlacedRuneInfo>& StartNode, TArray<int32>& CheckedIDs, TArray<int32>& ResultIDs);
};
