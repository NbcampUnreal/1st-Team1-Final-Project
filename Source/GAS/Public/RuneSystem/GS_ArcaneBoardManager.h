// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_ArcaneBoardTableRows.h"
#include "GS_ArcaneBoardTypes.h"
#include "GS_ArcaneBoardManager.generated.h"

// 스탯 변경 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatsChangedDelegate, const FCharacterStats&, NewStats);

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

	//현재 그리드에 배치된 룬들의 정보를 담은 배열
	UPROPERTY()
	TArray<FPlacedRuneInfo> PlacedRunes;

	UPROPERTY()
	FCharacterStats AppliedStatEffects;

	UPROPERTY()
	FCharacterStats CurrStatEffects;

	UPROPERTY()
	UDataTable* RuneTable;

	UPROPERTY()
	bool bHasUnsavedChanges;

	//스탯 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "ArcaneBoard|Events")
	FOnStatsChangedDelegate OnStatsChanged;

	//직업별 그리드 레이아웃 정보가 담긴 데이터 테이블 참조
	UPROPERTY()
	UDataTable* GridLayoutTable;

	UFUNCTION(BlueprintCallable)
	bool SetCurrClass(ECharacterClass NewClass);

	UFUNCTION(BlueprintCallable)
	bool CanPlaceRuneAt(uint8 RuneID, const FIntPoint& Pos);

	UFUNCTION(BlueprintCallable)
	bool PlaceRune(uint8 RuneID, const FIntPoint& Pos);

	UFUNCTION(BlueprintCallable)
	bool RemoveRune(uint8 RuneID);

	UFUNCTION(BlueprintCallable)
	FCharacterStats CalculateStatEffects();

	UFUNCTION(BlueprintCallable)
	void ApplyChanges();

	UFUNCTION(BlueprintCallable)
	void ResetAllRune();

	UFUNCTION(BlueprintCallable)
	void LoadSavedData(ECharacterClass Class, const TArray<FPlacedRuneInfo>& Runes, const FCharacterStats& Stats);

	UFUNCTION(BlueprintCallable)
	bool GetRuneData(uint8 RuneID, FRuneTableRow& OutData);

	UFUNCTION(BlueprintCallable)
	UGS_GridLayoutDataAsset* GetCurrGridLayout() const;

	UFUNCTION(BlueprintCallable)
	bool IsValidCell(const FIntPoint& Pos);

	UFUNCTION(BlueprintCallable)
	void InitDataCache();

	//UI 관련 함수
	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	bool PreviewRunePlacement(uint8 RuneID, const FIntPoint& Pos, TArray<FIntPoint>& OutAffectedCells);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void GetGridDimensions(int32& OutWidth, int32& OutHeight);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	bool GetCellData(const FIntPoint& Pos, FGridCellData& OutCellData);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	bool GetRuneShape(uint8 RuneID, TArray<FIntPoint>& OutShape);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	UTexture2D* GetRuneTexture(uint8 RuneID);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	bool GetFragmentedRuneTexture(uint8 RuneID, TMap<FIntPoint, UTexture2D*>& OutShape);

	UFUNCTION(BlueprintCallable, Category = "ArcaneBoard")
	void InitializeForTesting();

private:

	TMap<uint8, FRuneTableRow> RuneDataCache;
	TMap<ECharacterClass, UGS_GridLayoutDataAsset*> GridLayoutCache;
	
	//현재 클래스의 그리드 모양
	UPROPERTY()
	UGS_GridLayoutDataAsset* CurrGridLayout;

	//현재 그리드의 셀 상태
	TMap<FIntPoint, FGridCellData> CurrGridState;

	void InitGridState();

	void UpdateCellState(const FIntPoint& Pos, EGridCellState NewState);

	void UpdateCellState(const FIntPoint& Pos, EGridCellState NewState, uint8 RuneID, UTexture2D* RuneTextureFrag);

	//특수 셀과 연결된 룬 ID들의 배열
	UPROPERTY()
	TArray<uint8> ConnectedRuneIDs;

	//특수 셀과 룬의 연결 상태 업데이트
	void UpdateConnections();

	//시작점 룬에서부터 연결된 모든 룬을 재귀적으로 탐색
	void FindConnectedRunes(const TArray<FPlacedRuneInfo>& StartNode, TArray<uint8>& CheckedIDs, TArray<uint8>& ResultIDs);

	bool IsRuneAdjacentToCell(const FPlacedRuneInfo& Rune, const FIntPoint& CellPos) const;
	bool AreRunesAdjacent(const FPlacedRuneInfo& Rune1, const FPlacedRuneInfo& Rune2) const;
	void ApplySpecialCellBonus(TMap<FName, float>& StatEffects);

	//실제 변경사항 여부 확인 함수
	bool HasActualChanges() const;
};
