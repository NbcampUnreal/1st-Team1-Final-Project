// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "Engine/DataTable.h"

UGS_ArcaneBoardManager::UGS_ArcaneBoardManager()
{
	CurrClass = ECharacterClass::Ares;
	CurrGridLayout = nullptr;

	RuneTable = nullptr;
	GridLayoutTable = nullptr;
}

bool UGS_ArcaneBoardManager::SetCurrClass(ECharacterClass NewClass)
{
	CurrClass = NewClass;

	//캐시된 그리드 레이아웃이 있는지 확인
	if (GridLayoutCache.Contains(NewClass))
	{
		CurrGridLayout = GridLayoutCache[NewClass];
		return true;
	}

	//캐시에 없으면 그리드 레이아웃 데이터 로드
	if (GridLayoutTable)
	{
		FString RowName = UGS_EnumUtils::GetEnumAsString(NewClass);
		FGridLayoutTableRow* LayoutRow = GridLayoutTable->FindRow<FGridLayoutTableRow>(*RowName, TEXT("SetCurrClass"));

		if (LayoutRow && LayoutRow->GridLayoutAsset.IsValid())
		{
			//그리드 레이아웃 에셋 로드 및 캐싱
			UGS_GridLayoutDataAsset* LayoutAsset = LayoutRow->GridLayoutAsset.LoadSynchronous();
			if (LayoutAsset)
			{
				GridLayoutCache.Add(NewClass, LayoutAsset);
				CurrGridLayout = LayoutAsset;
				return true;
			}
		}
	}

	//레이아웃을 찾지 못했거나 로드에 실패한 경우
	CurrGridLayout = nullptr;
	return false;
}

bool UGS_ArcaneBoardManager::CanPlaceRuneAt(int32 RuneID, const FIntPoint& Pos)
{
	return false;
}

bool UGS_ArcaneBoardManager::PlaceRune(int32 RuneID, const FIntPoint& Pos)
{
	return false;
}

bool UGS_ArcaneBoardManager::RemoveRune(int32 RuneID)
{
	return false;
}

TMap<FName, float> UGS_ArcaneBoardManager::CalculateStatEffects()
{
	return TMap<FName, float>();
}

void UGS_ArcaneBoardManager::ApplyChanges()
{
}

void UGS_ArcaneBoardManager::ResetAllRune()
{
}

void UGS_ArcaneBoardManager::LoadSavedData(ECharacterClass Class, const TArray<FPlacedRuneInfo>& Runes, const TMap<FName, float>& Stats)
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
	if (RuneTable)
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
	if (!CurrGridLayout)
	{
		return false;
	}

	//그리드 범위 밖이면 무효
	if (Pos.X < 0 || Pos.X >= CurrGridLayout->GridSize.X ||
		Pos.Y < 0 || Pos.Y >= CurrGridLayout->GridSize.Y)
	{
		return false;
	}

	//해당 위치가 유효한 셀인지 확인
	for (const FGridCellData& Cell : CurrGridLayout->GridCells)
	{
		if (Cell.Pos == Pos && Cell.State != EGridCellState::Locked)
		{
			return true;
		}
	}

	return false;
}

void UGS_ArcaneBoardManager::InitDataCache()
{
	RuneDataCache.Empty();
	GridLayoutCache.Empty();

	//룬 데이터 캐싱
	if (RuneTable)
	{
		TArray<FName> RowNames = RuneTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FRuneTableRow* Row = RuneTable->FindRow<FRuneTableRow>(RowName, TEXT("InitDataCache"));
			if (Row)
			{
				int32 RuneID = FCString::Atoi(*RowName.ToString());
				if (RuneID > 0)
				{
					RuneDataCache.Add(RuneID, *Row);
				}
			}
		}
	}

	//그리드 레이아웃 캐싱
	if (GridLayoutTable)
	{
		TArray<FName> RowNames = GridLayoutTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FGridLayoutTableRow* Row = GridLayoutTable->FindRow<FGridLayoutTableRow>(RowName, TEXT("InitDataCache"));
			if (Row && Row->GridLayoutAsset.IsValid())
			{
				UGS_GridLayoutDataAsset* Asset = Row->GridLayoutAsset.LoadSynchronous();
				if (Asset)
				{
					int EnumCnt = UGS_EnumUtils::GetEnumCount<ECharacterClass>();
					for (int32 i = 0; i < EnumCnt; i++)
					{
						ECharacterClass Class = static_cast<ECharacterClass>(i);
						FString EnumStr = TEXT("ECharacterClass::") + UGS_EnumUtils::GetEnumAsString(Class);
						
						if (RowName.ToString() == EnumStr)
						{
							GridLayoutCache.Add(Class, Asset);
							break;
						}
					}
				}
			}
		}
	}

	if (GridLayoutCache.Contains(CurrClass))
	{
		CurrGridLayout = GridLayoutCache[CurrClass];
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
