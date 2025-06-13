// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "Engine/DataTable.h"

UGS_ArcaneBoardManager::UGS_ArcaneBoardManager()
{
	//기본 초기화
	CurrClass = ECharacterClass::Ares;
	PlacedRunes.Empty();
	AppliedStatEffects = FGS_StatRow();
	CurrStatEffects = FGS_StatRow();
	CurrGridLayout = nullptr;

	//데이터 테이블은 ArcaneBoardLPS에서 설정
	/*RuneTable = nullptr;
	GridLayoutTable = nullptr;*/

	//임시
	static ConstructorHelpers::FObjectFinder<UDataTable> RuneTableFinder(TEXT("/Game/DataTable/RuneSystem/DT_RuneDataTable"));
	if (RuneTableFinder.Succeeded())
	{
		RuneTable = RuneTableFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> GridLayoutTableFinder(TEXT("/Game/DataTable/RuneSystem/DT_GridLayoutTable"));
	if (GridLayoutTableFinder.Succeeded())
	{
		GridLayoutTable = GridLayoutTableFinder.Object;
	}

	InitDataCache();
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

	AppliedStatEffects = CalculateStatEffects();
	CurrStatEffects = AppliedStatEffects;

	return true;
}

ECharacterClass UGS_ArcaneBoardManager::GetCurrClass()
{
	return CurrClass;
}

bool UGS_ArcaneBoardManager::CanPlaceRuneAt(uint8 RuneID, const FIntPoint& Pos)
{
	TArray<FIntPoint> RuneShape;
	if (!GetRuneShape(RuneID, RuneShape))
	{
		return false;
	}

	for (const FIntPoint& Offset : RuneShape)
	{
		FIntPoint CellPos = Pos + Offset;
		if (!IsValidCell(CellPos))
		{
			return false;
		}
	}

	return true;
}

bool UGS_ArcaneBoardManager::FindOptimalPlacementPos(uint8 RuneID, const FIntPoint& ClickedCell, FIntPoint& OutBestPos)
{
	TArray<FIntPoint> RuneShape;
	if (!GetRuneShape(RuneID, RuneShape))
	{
		return false;
	}

	TArray<FIntPoint> ValidPos;

	for (const FIntPoint& ShapeOffset : RuneShape)
	{
		FIntPoint CandidatePos = ClickedCell - ShapeOffset;

		if (CanPlaceRuneAt(RuneID, CandidatePos))
		{
			ValidPos.Add(CandidatePos);
		}
	}

	if (ValidPos.Num() == 0)
	{
		return false;
	}

	OutBestPos = ValidPos[0];
	FIntPoint BestShapeOffset = ClickedCell - ValidPos[0];

	for (int32 i = 1; i < ValidPos.Num(); ++i)
	{
		FIntPoint Pos = ValidPos[i];
		FIntPoint ShapeOffset = ClickedCell - Pos;

		bool bIsBetter = false;

		if (ShapeOffset.Y < BestShapeOffset.Y)
		{
			bIsBetter = true;
		}
		else if (ShapeOffset.Y == BestShapeOffset.Y && ShapeOffset.X < BestShapeOffset.X)
		{
			bIsBetter = true;
		}

		if (bIsBetter)
		{
			OutBestPos = Pos;
			BestShapeOffset = ShapeOffset;
		}
	}

	return true;
}

bool UGS_ArcaneBoardManager::PlaceRune(uint8 RuneID, const FIntPoint& Pos)
{
	if (!CanPlaceRuneAt(RuneID, Pos))
	{
		return false;
	}

	TMap<FIntPoint, UTexture2D*> RuneShape;
	if (!GetFragmentedRuneTexture(RuneID, RuneShape))
	{
		return false;
	}

	//룬 배치 정보 저장
	FPlacedRuneInfo NewRune(RuneID, Pos);
	PlacedRunes.Add(NewRune);
	
	for(const FPlacedRuneInfo& rune : PlacedRunes)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d"), rune.RuneID);
	}

	ApplyRuneToGrid(RuneID, Pos, EGridCellState::Occupied, true);

	bHasUnsavedChanges = true;

	CurrStatEffects = CalculateStatEffects();

	//스탯창 UI 업데이트 이벤트 호출
	OnStatsChanged.Broadcast(CurrStatEffects);

	return true;
}

bool UGS_ArcaneBoardManager::RemoveRune(uint8 RuneID)
{
	//배치된 룬 목록에서 해당 룬 찾기
	int8 RuneIndex = INDEX_NONE;
	for (uint8 i = 0; i < PlacedRunes.Num(); ++i)
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

	FIntPoint RunePos = PlacedRunes[RuneIndex].Pos;
	ApplyRuneToGrid(RuneID, RunePos, EGridCellState::Empty, false);

	PlacedRunes.RemoveAt(RuneIndex);

	bHasUnsavedChanges = true;

	CurrStatEffects = CalculateStatEffects();

	//스탯창 UI 업데이트 이벤트 호출
	OnStatsChanged.Broadcast(CurrStatEffects);

	return true;
}

FGS_StatRow UGS_ArcaneBoardManager::CalculateStatEffects()
{
	FGS_StatRow Result;

	for (const FPlacedRuneInfo& Rune : PlacedRunes)
	{
		FRuneTableRow RuneData;
		if (GetRuneData(Rune.RuneID, RuneData))
		{
			if (RuneData.StatEffect.StatName == FName("HP"))
			{
				Result.HP += RuneData.StatEffect.Value;
			}
			else if (RuneData.StatEffect.StatName == FName("ATK"))
			{
				Result.ATK += RuneData.StatEffect.Value;
			}
			else if (RuneData.StatEffect.StatName == FName("DEF"))
			{
				Result.DEF += RuneData.StatEffect.Value;
			}
			else if (RuneData.StatEffect.StatName == FName("AGL"))
			{
				Result.AGL += RuneData.StatEffect.Value;
			}
			else if (RuneData.StatEffect.StatName == FName("ATS"))
			{
				Result.ATS += RuneData.StatEffect.Value;
			}
		}
	}

	// 특수 셀 연결로직

	return Result;
}

void UGS_ArcaneBoardManager::ApplyChanges()
{
	AppliedStatEffects = CurrStatEffects;

	bHasUnsavedChanges = false;

	OnStatsChanged.Broadcast(AppliedStatEffects);
}

void UGS_ArcaneBoardManager::ResetAllRune()
{
	if (PlacedRunes.Num() == 0)
	{
		return;
	}

	PlacedRunes.Empty();
	InitGridState();
	CurrStatEffects = FGS_StatRow();
	bHasUnsavedChanges = true;
	OnStatsChanged.Broadcast(CurrStatEffects);
}

void UGS_ArcaneBoardManager::LoadSavedData(ECharacterClass Class, const TArray<FPlacedRuneInfo>& Runes)
{
	PlacedRunes.Empty();
	InitGridState();

	for (const FPlacedRuneInfo& RuneInfo : Runes)
	{
		PlacedRunes.Add(RuneInfo);
		ApplyRuneToGrid(RuneInfo.RuneID, RuneInfo.Pos, EGridCellState::Occupied, true);
	}

	AppliedStatEffects = CalculateStatEffects();
	CurrStatEffects = AppliedStatEffects;

	OnStatsChanged.Broadcast(CurrStatEffects);

	bHasUnsavedChanges = false;
}

bool UGS_ArcaneBoardManager::GetRuneData(uint8 RuneID, FRuneTableRow& OutData)
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

bool UGS_ArcaneBoardManager::PreviewRunePlacement(uint8 RuneID, const FIntPoint& Pos, TArray<FIntPoint>& OutAffectedCells)
{
	OutAffectedCells.Empty();

	TArray<FIntPoint> RuneShape;
	if (!GetRuneShape(RuneID, RuneShape))
	{
		return false;
	}

	bool bCanPlace = true;
	for (const FIntPoint& Offset : RuneShape)
	{
		FIntPoint CellPos(Pos.X + Offset.X, Pos.Y + Offset.Y);
		OutAffectedCells.Add(CellPos);

		if (!IsValidCell(CellPos))
		{
			bCanPlace = false;
		}
	}

	return bCanPlace;
}

void UGS_ArcaneBoardManager::GetGridDimensions(int32& OutRows, int32& OutColumns)
{
	if (IsValid(CurrGridLayout))
	{
		OutRows = CurrGridLayout->GridSize.X;
		OutColumns = CurrGridLayout->GridSize.Y;
	}
	else
	{
		OutRows = 0;
		OutColumns = 0;
	}
}

bool UGS_ArcaneBoardManager::GetCellData(const FIntPoint& Pos, FGridCellData& OutCellData)
{
	if (CurrGridState.Contains(Pos))
	{
		OutCellData = CurrGridState[Pos];
		return true;
	}

	return false;
}

bool UGS_ArcaneBoardManager::GetRuneShape(uint8 RuneID, TArray<FIntPoint>& OutShape)
{
	FRuneTableRow RuneData;
	if (GetRuneData(RuneID, RuneData))
	{
		RuneData.RuneShape.GenerateKeyArray(OutShape);
		return true;
	}

	return false;
}

UTexture2D* UGS_ArcaneBoardManager::GetRuneTexture(uint8 RuneID)
{
	FRuneTableRow RuneData;
	if (GetRuneData(RuneID, RuneData))
	{
		return RuneData.RuneTexture.LoadSynchronous();
	}

	return nullptr;
}

bool UGS_ArcaneBoardManager::GetFragmentedRuneTexture(uint8 RuneID, TMap<FIntPoint, UTexture2D*>& OutShape)
{
	FRuneTableRow RuneData;
	if (GetRuneData(RuneID, RuneData))
	{
		OutShape = RuneData.RuneShape;
		return true;
	}

	return false;
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
			else if (NewCell.State == EGridCellState::Occupied)
			{
				PlacedRunes.Add(FPlacedRuneInfo(Cell.PlacedRuneID, Cell.Pos));
			}
			CurrGridState.Add(Cell.Pos, NewCell);
		}
	}
}

void UGS_ArcaneBoardManager::ApplyRuneToGrid(uint8 RuneID, const FIntPoint& Position, EGridCellState NewState, bool bApplyTexture)
{
	TMap<FIntPoint, UTexture2D*> RuneShape;
	if (!GetFragmentedRuneTexture(RuneID, RuneShape))
	{
		return;
	}

	for (const auto& ShapePair : RuneShape)
	{
		FIntPoint CellPos(Position.X + ShapePair.Key.X, Position.Y + ShapePair.Key.Y);

		UTexture2D* TextureToApply = nullptr;
		uint8 RuneIDToApply = 0;

		if (NewState == EGridCellState::Occupied && bApplyTexture)
		{
			TextureToApply = ShapePair.Value;
			RuneIDToApply = RuneID;
		}

		UpdateCellState(CellPos, NewState, RuneIDToApply, TextureToApply);
	}
}

void UGS_ArcaneBoardManager::UpdateCellState(const FIntPoint& Pos, EGridCellState NewState, uint8 RuneID, UTexture2D* RuneTextureFrag)
{
	if (CurrGridState.Contains(Pos))
	{
		CurrGridState[Pos].State = NewState;
		CurrGridState[Pos].PlacedRuneID = RuneID;
		CurrGridState[Pos].RuneTextureFrag = RuneTextureFrag;
	}
}

void UGS_ArcaneBoardManager::UpdateConnections()
{
}

void UGS_ArcaneBoardManager::FindConnectedRunes(const TArray<FPlacedRuneInfo>& StartNode, TArray<uint8>& CheckedIDs, TArray<uint8>& ResultIDs)
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
