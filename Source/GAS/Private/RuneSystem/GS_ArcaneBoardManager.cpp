// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "Engine/DataTable.h"

UGS_ArcaneBoardManager::UGS_ArcaneBoardManager()
{
	//기본 초기화
	CurrClass = ECharacterClass::Ares;
	PlacedRunes.Empty();
	AppliedStatEffects = FCharacterStats();
	CurrStatEffects = FCharacterStats();
	CurrGridLayout = nullptr;

	// 데이터 테이블은 ArcaneBoardLPS에서 설정
	RuneTable = nullptr;
	GridLayoutTable = nullptr;
}

bool UGS_ArcaneBoardManager::SetCurrClass(ECharacterClass NewClass)
{
	if (CurrClass == NewClass && IsValid(CurrGridLayout))
	{
		return true;
	}

	if (!GridLayoutCache.Contains(NewClass))
	{
		//캐시에 없으면 데이터 테이블에서 직접 로드
		if (IsValid(GridLayoutTable))
		{
			FString RowName = UGS_EnumUtils::GetEnumAsString(NewClass);
			FGridLayoutTableRow* LayoutRow = GridLayoutTable->FindRow<FGridLayoutTableRow>(*RowName, TEXT("SetCurrClass"));

			if (LayoutRow && !LayoutRow->GridLayoutAsset.IsNull())
			{
				UGS_GridLayoutDataAsset* LayoutAsset = LayoutRow->GridLayoutAsset.LoadSynchronous();
				if (LayoutAsset)
				{
					GridLayoutCache.Add(NewClass, LayoutAsset);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("그리드 레이아웃 에셋 로드 실패: %s"), *RowName);
					return false;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("그리드 레이아웃 데이터 없음: %s"), *RowName);
				return false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GridLayoutTable이 유효하지 않음"));
			return false;
		}
	}

	CurrClass = NewClass;
	CurrGridLayout = GridLayoutCache[NewClass];

	InitGridState();

	bHasUnsavedChanges = false;

	CurrStatEffects = FCharacterStats();
	AppliedStatEffects = FCharacterStats();

	//후에 저장된 룬 배치 로드해야 함

	return true;
}

bool UGS_ArcaneBoardManager::CanPlaceRuneAt(int32 RuneID, const FIntPoint& Pos)
{
	//룬 데이터 로드
	FRuneTableRow RuneData;
	if (!GetRuneData(RuneID, RuneData))
	{
		return false;
	}

	//룬이 차지하는 모든 셀 확인
	for (const FIntPoint& Offset : RuneData.RuneShape)
	{
		FIntPoint CellPos(Pos.X + Offset.X, Pos.Y + Offset.Y);

		if (!IsValidCell(CellPos))
		{
			return false;
		}
	}

	return true;
}

bool UGS_ArcaneBoardManager::PlaceRune(int32 RuneID, const FIntPoint& Pos)
{
	if (!CanPlaceRuneAt(RuneID, Pos))
	{
		return false;
	}

	FRuneTableRow RuneData;
	if (!GetRuneData(RuneID, RuneData))
	{
		return false;
	}

	//룬 배치 정보 저장
	FPlacedRuneInfo NewRune(RuneID, Pos);
	PlacedRunes.Add(NewRune);

	//셀 상태 업데이트
	for (const FIntPoint& Offset : RuneData.RuneShape)
	{
		FIntPoint CellPos(Pos.X + Offset.X, Pos.Y + Offset.Y);
		UpdateCellState(CellPos, EGridCellState::Occupied, RuneID);
	}

	bHasUnsavedChanges = true;

	CurrStatEffects = CalculateStatEffects();

	//스탯창 UI 업데이트 이벤트 호출
	OnStatsChanged.Broadcast(CurrStatEffects);

	return true;
}

bool UGS_ArcaneBoardManager::RemoveRune(int32 RuneID)
{
	//배치된 룬 목록에서 해당 룬 찾기
	int32 RuneIndex = INDEX_NONE;
	for (int32 i = 0; i < PlacedRunes.Num(); ++i)
	{
		if (PlacedRunes[i].RuneID == RuneID)
		{
			RuneIndex = i;
			break;
		}
	}

	if (RuneID == INDEX_NONE)
	{
		return false;
	}

	FRuneTableRow RuneData;
	if (!GetRuneData(RuneID, RuneData))
	{
		return false;
	}

	FIntPoint RunePos = PlacedRunes[RuneIndex].Pos;
	for (const FIntPoint& Offset : RuneData.RuneShape)
	{
		FIntPoint CellPos(RunePos.X + Offset.X, RunePos.X + Offset.X);
		UpdateCellState(CellPos, EGridCellState::Empty);
	}

	PlacedRunes.RemoveAt(RuneIndex);

	bHasUnsavedChanges = true;

	CurrStatEffects = CalculateStatEffects();

	//스탯창 UI 업데이트 이벤트 호출
	OnStatsChanged.Broadcast(CurrStatEffects);

	return false;
}

FCharacterStats UGS_ArcaneBoardManager::CalculateStatEffects()
{
	FCharacterStats Result;

	for (const FPlacedRuneInfo& Rune : PlacedRunes)
	{
		FRuneTableRow RuneData;
		if (GetRuneData(Rune.RuneID, RuneData))
		{
			if (RuneData.FStatEffect.StatName == FName("MaxHP"))
			{
				Result.MaxHP += RuneData.FStatEffect.Value;
			}
			else if (RuneData.FStatEffect.StatName == FName("ATK"))
			{
				Result.ATK += RuneData.FStatEffect.Value;
			}
			else if (RuneData.FStatEffect.StatName == FName("DEF"))
			{
				Result.DEF += RuneData.FStatEffect.Value;
			}
			else if (RuneData.FStatEffect.StatName == FName("AGL"))
			{
				Result.AGL += RuneData.FStatEffect.Value;
			}
			else if (RuneData.FStatEffect.StatName == FName("ATS"))
			{
				Result.ATS += RuneData.FStatEffect.Value;
			}
		}
	}

	return Result;
}

void UGS_ArcaneBoardManager::ApplyChanges()
{
}

void UGS_ArcaneBoardManager::ResetAllRune()
{
}

void UGS_ArcaneBoardManager::LoadSavedData(ECharacterClass Class, const TArray<FPlacedRuneInfo>& Runes, const FCharacterStats& Stats)
{
}

bool UGS_ArcaneBoardManager::GetRuneData(int32 RuneID, FRuneTableRow& OutData)
{
	//캐시에서 룬 데이터 찾기
	if (RuneDataCache.Contains(RuneID))
	{
		OutData = RuneDataCache[RuneID];
		return true;
	}

	//캐시에 없으면 테이블에서 찾기
	if (IsValid(RuneTable))
	{
		FString RowName = FString::FromInt(RuneID);
		FRuneTableRow* FoundRow = RuneTable->FindRow<FRuneTableRow>(*RowName, TEXT("GetRuneData"));

		if (FoundRow)
		{
			//데이터 캐싱
			RuneDataCache.Add(RuneID, *FoundRow);
			OutData = *FoundRow;
			return true;
		}
	}

	return false;
}

UGS_GridLayoutDataAsset* UGS_ArcaneBoardManager::GetCurrGridLayout() const
{
	return CurrGridLayout;
}

bool UGS_ArcaneBoardManager::IsValidCell(const FIntPoint& Pos)
{
	if (!IsValid(CurrGridLayout))
	{
		return false;
	}

	if (CurrGridState.Contains(Pos))
	{
		return CurrGridState[Pos].State == EGridCellState::Empty;
	}

	return false;
}

void UGS_ArcaneBoardManager::InitDataCache()
{
	RuneDataCache.Empty();
	GridLayoutCache.Empty();

	//룬 데이터 캐싱
	if (IsValid(RuneTable))
	{
		TArray<FRuneTableRow*> RuneRows;
		RuneTable->GetAllRows<FRuneTableRow>(TEXT("InitDataCache"), RuneRows);

		for (FRuneTableRow* Row : RuneRows)
		{
			if (Row)
			{
				RuneDataCache.Add(Row->RuneID, *Row);
			}
		}
	}

	//그리드 레이아웃 캐싱
	if (IsValid(GridLayoutTable))
	{
		TArray<FGridLayoutTableRow*> GridRows;
		GridLayoutTable->GetAllRows<FGridLayoutTableRow>(TEXT("InitDataCache"), GridRows);

		for (FGridLayoutTableRow* Row : GridRows)
		{
			if (Row && !Row->GridLayoutAsset.IsNull())
			{
				UGS_GridLayoutDataAsset* LoadedAsset = Row->GridLayoutAsset.LoadSynchronous();
				if (LoadedAsset)
				{
					GridLayoutCache.Add(LoadedAsset->CharacterClass, LoadedAsset);
				}
			}
		}
	}

	//초기 클래스 설정
	SetCurrClass(CurrClass);
}

void UGS_ArcaneBoardManager::InitGridState()
{
	CurrGridState.Empty();

	if (IsValid(CurrGridLayout))
	{
		for (const FGridCellData& Cell : CurrGridLayout->GridCells)
		{
			FGridCellData NewCell = Cell;
			if (NewCell.State == EGridCellState::Empty)
			{
				NewCell.PlacedRuneID = 0;
			}
			CurrGridState.Add(Cell.Pos, NewCell);
		}
	}
}

void UGS_ArcaneBoardManager::UpdateCellState(const FIntPoint& Pos, EGridCellState NewState, uint8 RuneID)
{
	if (CurrGridState.Contains(Pos))
	{
		CurrGridState[Pos].State = NewState;

		if (NewState == EGridCellState::Occupied)
		{
			CurrGridState[Pos].PlacedRuneID = RuneID;
		}
		else
		{
			CurrGridState[Pos].PlacedRuneID = 0;
		}
	}
}

void UGS_ArcaneBoardManager::UpdateConnections()
{
}

void UGS_ArcaneBoardManager::FindConnectedRunes(const TArray<FPlacedRuneInfo>& StartNode, TArray<int32>& CheckedIDs, TArray<int32>& ResultIDs)
{
}

bool UGS_ArcaneBoardManager::IsRuneAdjacentToCell(const FPlacedRuneInfo& Rune, const FIntPoint& CellPos) const
{
	return false;
}

bool UGS_ArcaneBoardManager::AreRunesAdjacent(const FPlacedRuneInfo& Rune1, const FPlacedRuneInfo& Rune2) const
{
	return false;
}

void UGS_ArcaneBoardManager::ApplySpecialCellBonus(TMap<FName, float>& StatEffects)
{
}

bool UGS_ArcaneBoardManager::HasActualChanges() const
{
	return false;
}
