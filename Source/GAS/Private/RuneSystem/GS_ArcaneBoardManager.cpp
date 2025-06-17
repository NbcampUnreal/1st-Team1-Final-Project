// Copyright Epic Games, Inc. All Rights Reserved.

#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_GridLayoutDataAsset.h"
#include "Engine/DataTable.h"

UGS_ArcaneBoardManager::UGS_ArcaneBoardManager()
{
	//기본 초기화
	CurrClass = ECharacterClass::Ares;
	PlacedRunes.Empty();
	AppliedBoardStats = FArcaneBoardStats();
	CurrBoardStats = FArcaneBoardStats();
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

	bool bNeedGridReset = (CurrClass != NewClass);

	CurrClass = NewClass;
	CurrGridLayout = GridLayoutCache[NewClass];

	if (bNeedGridReset)
	{
		InitGridState();
		CalculateStatEffects();
		AppliedBoardStats = CurrBoardStats;
	}
	
	bHasUnsavedChanges = false;

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

	CalculateStatEffects();

	//스탯창 UI 업데이트 이벤트 호출
	OnStatsChanged.Broadcast(CurrBoardStats);

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

	CalculateStatEffects();

	//스탯창 UI 업데이트 이벤트 호출
	OnStatsChanged.Broadcast(CurrBoardStats);

	return true;
}

void UGS_ArcaneBoardManager::CalculateStatEffects()
{
	//특수 셀 연결 업데이트
	UpdateConnections();

	FGS_StatRow BaseResult, BonusResult;
	float BonusValue = ConnectedRuneCnt;

	for (const FPlacedRuneInfo& Rune : PlacedRunes)
	{
		FRuneTableRow RuneData;
		if (GetRuneData(Rune.RuneID, RuneData))
		{
			bool bIsRuneConnected = IsRuneConnected(Rune.RuneID);

			if (RuneData.StatEffect.StatName == FName("HP"))
			{
				BaseResult.HP += RuneData.StatEffect.Value;

				if (bIsRuneConnected && BonusResult.HP == 0)
				{
					BonusResult.HP = BonusValue;
				}
			}
			else if (RuneData.StatEffect.StatName == FName("ATK"))
			{
				BaseResult.ATK += RuneData.StatEffect.Value;

				if (bIsRuneConnected && BonusResult.ATK == 0)
				{
					BonusResult.ATK = BonusValue;
				}
			}
			else if (RuneData.StatEffect.StatName == FName("DEF"))
			{
				BaseResult.DEF += RuneData.StatEffect.Value;
				
				if (bIsRuneConnected && BonusResult.DEF == 0)
				{
					BonusResult.DEF = BonusValue;
				}
			}
			else if (RuneData.StatEffect.StatName == FName("AGL"))
			{
				BaseResult.AGL += RuneData.StatEffect.Value;

				if (bIsRuneConnected && BonusResult.AGL == 0)
				{
					BonusResult.AGL = BonusValue;
				}
			}
			else if (RuneData.StatEffect.StatName == FName("ATS"))
			{
				BaseResult.ATS += RuneData.StatEffect.Value;

				if (bIsRuneConnected && BonusResult.ATS == 0)
				{
					BonusResult.ATS = BonusValue;
				}
			}
		}
	}

	CurrBoardStats.RuneStats = BaseResult;
	CurrBoardStats.BonusStats = BonusResult;
}

void UGS_ArcaneBoardManager::ApplyChanges()
{
	AppliedBoardStats = CurrBoardStats;

	bHasUnsavedChanges = false;

	OnStatsChanged.Broadcast(AppliedBoardStats);
}

void UGS_ArcaneBoardManager::ResetAllRune()
{
	if (PlacedRunes.Num() == 0)
	{
		return;
	}

	PlacedRunes.Empty();
	InitGridState();
	CurrBoardStats = FArcaneBoardStats();
	bHasUnsavedChanges = true;
	OnStatsChanged.Broadcast(CurrBoardStats);
}

void UGS_ArcaneBoardManager::LoadSavedData(ECharacterClass Class, const TArray<FPlacedRuneInfo>& Runes)
{
	PlacedRunes.Empty();
	InitGridState();
	CurrBoardStats = FArcaneBoardStats();

	for (const FPlacedRuneInfo& RuneInfo : Runes)
	{
		PlacedRunes.Add(RuneInfo);
		ApplyRuneToGrid(RuneInfo.RuneID, RuneInfo.Pos, EGridCellState::Occupied, true);
	}

	CalculateStatEffects();
	AppliedBoardStats = CurrBoardStats;

	OnStatsChanged.Broadcast(CurrBoardStats);

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

bool UGS_ArcaneBoardManager::GetConnectedFragmentedRuneTexture(uint8 RuneID, TMap<FIntPoint, UTexture2D*>& OutShape)
{
	FRuneTableRow RuneData;
	if (GetRuneData(RuneID, RuneData))
	{
		OutShape = RuneData.ConnectedRuneShape;
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

			if (Cell.bIsSpecialCell)
			{
				SpecialCellPos = Cell.Pos;
			}
		}
	}
}

void UGS_ArcaneBoardManager::ApplyRuneToGrid(uint8 RuneID, const FIntPoint& Position, EGridCellState NewState, bool bApplyTexture)
{
	TMap<FIntPoint, UTexture2D*> RuneShape;
	TMap<FIntPoint, UTexture2D*> ConnectedRuneShape;

	if (!GetFragmentedRuneTexture(RuneID, RuneShape))
	{
		return;
	}

	if (!GetConnectedFragmentedRuneTexture(RuneID, ConnectedRuneShape))
	{
		return;
	}

	for (const auto& ShapePair : RuneShape)
	{
		FIntPoint CellPos(Position.X + ShapePair.Key.X, Position.Y + ShapePair.Key.Y);

		UTexture2D* NormalTexture = nullptr;
		UTexture2D* ConnectedTexture = nullptr;
		uint8 RuneIDToApply = 0;

		if (NewState == EGridCellState::Occupied && bApplyTexture)
		{
			NormalTexture = ShapePair.Value;
			RuneIDToApply = RuneID;

			// 연결 상태 텍스처도 설정
			if (ConnectedRuneShape.Contains(ShapePair.Key))
			{
				ConnectedTexture = ConnectedRuneShape[ShapePair.Key];
			}
		}

		UpdateCellState(CellPos, NewState, RuneIDToApply, NormalTexture, ConnectedTexture);
	}
}

void UGS_ArcaneBoardManager::UpdateCellState(const FIntPoint& Pos, EGridCellState NewState, uint8 RuneID, UTexture2D* RuneTextureFrag, UTexture2D* ConnectedRuneTextureFrag)
{
	if (CurrGridState.Contains(Pos))
	{
		CurrGridState[Pos].State = NewState;
		CurrGridState[Pos].PlacedRuneID = RuneID;
		CurrGridState[Pos].RuneTextureFrag = RuneTextureFrag;
		CurrGridState[Pos].ConnectedRuneTextureFrag = ConnectedRuneTextureFrag;
	}
}

void UGS_ArcaneBoardManager::UpdateConnections()
{
	for (auto& CellPair : CurrGridState)
	{
		CellPair.Value.bIsConnected = false;
	}

	TSet<FIntPoint> VisitedCells;
	FindConnectedCells(SpecialCellPos, VisitedCells);

	TSet<uint8> ConnectedRuneIDs;
	for (const auto& CellPair : CurrGridState)
	{
		if (CellPair.Value.bIsConnected && CellPair.Value.PlacedRuneID > 0)
		{
			ConnectedRuneIDs.Add(CellPair.Value.PlacedRuneID);
		}
	}

	ConnectedRuneCnt = ConnectedRuneIDs.Num();
}

void UGS_ArcaneBoardManager::FindConnectedCells(const FIntPoint CellPos, TSet<FIntPoint>& VisitedCell)
{
	if (VisitedCell.Contains(CellPos))
	{
		return;
	}

	FGridCellData CellData;
	if (!GetCellData(CellPos, CellData) || CellData.State == EGridCellState::Empty)
	{
		return;
	}

	VisitedCell.Add(CellPos);
	if (CurrGridState.Contains(CellPos))
	{
		CurrGridState[CellPos].bIsConnected = true;
	}

	TArray<FIntPoint> Directions = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
	for (const FIntPoint& Dir : Directions)
	{
		FIntPoint NextPos = CellPos + Dir;
		FindConnectedCells(NextPos, VisitedCell);
	}
}

bool UGS_ArcaneBoardManager::IsRuneConnected(uint8 RuneID)
{
	for (const auto& CellPair : CurrGridState)
	{
		if (CellPair.Value.PlacedRuneID == RuneID && CellPair.Value.bIsConnected)
		{
			return true;
		}
	}
	return false;
}
