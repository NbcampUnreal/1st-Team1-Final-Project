#include "DungeonEditor/GS_BuildManager.h"

#include "EngineUtils.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/BillboardComponent.h"
#include "Components/DecalComponent.h"
#include "DungeonEditor/GS_DEController.h"
#include "DungeonEditor/Component/PlaceInfoComponent.h"
#include "DungeonEditor/Data/GS_DungeonEditorSaveGame.h"
#include "DungeonEditor/Data/GS_PlaceableObjectsRow.h"
#include "Kismet/GameplayStatics.h"
#include "Props/GS_RoomBase.h"
#include "Props/Placer/GS_PlacerBase.h"

AGS_BuildManager::AGS_BuildManager()
{
	PrimaryActorTick.bCanEverTick = true;

#if WITH_EDITORONLY_DATA
	BillboardCompo = CreateDefaultSubobject<UBillboardComponent>("BillBoard");
	SetRootComponent(BillboardCompo);
#endif
	StaticMeshCompo = CreateDefaultSubobject<UStaticMeshComponent>("GridMesh");
	StaticMeshCompo->SetupAttachment(RootComponent);
	DecalCompo = CreateDefaultSubobject<UDecalComponent>("Decal");
	DecalCompo->SetupAttachment(RootComponent);
	
	bTraceComplex = false;
	StartTraceHeight = 3000;
	EndTraceHeight = -3000;
	GridSize = {32, 32};
	CellSize = 100.0f;
	GridColor = {0.0f, 0.853f, 1.0f, 1.0f};
	GridOpacity = 50.0f;

	bIsCellUnderCursorChanged = false;

	CurrentSaveSlotName = TEXT("Preset_0");
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Grid/MI_Grid.MI_Grid'"));
	if (MatFinder.Succeeded())
	{
		GridMaterial = MatFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture> TexFinder(
		TEXT("Texture2D'/Game/DungeonEditor/Textures/T_Grid_Cell.T_Grid_Cell'"));
	if (TexFinder.Succeeded())
	{
		GridTexture = TexFinder.Object; // GridTexture에 할당
	}
	
	// InitGrid();
}

void AGS_BuildManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	InitGrid();
#if WITH_EDITOR
	if (nullptr != MIDGrid)
	{
		StaticMeshCompo->SetMaterial(0, MIDGrid);
	}
#endif
}

void AGS_BuildManager::BeginPlay()
{
	Super::BeginPlay();
	InitGrid();
	if (nullptr != MIDGrid)
	{
		StaticMeshCompo->SetMaterial(0, MIDGrid);
	}

	if (DecalCompo)
	{
		UMaterialInterface* OriginalMaterial = DecalCompo->GetMaterial(0);
		if (OriginalMaterial)
		{
			DecalMaterialInstance = UMaterialInstanceDynamic::Create(OriginalMaterial, this);
			if (DecalMaterialInstance)
			{
				DecalCompo->SetMaterial(0, DecalMaterialInstance);
				DecalMaterialInstance->SetScalarParameterValue("ShowType", 7);
			}
		}
	}
}

void AGS_BuildManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bBuildToolEnabled || bDemolitionToolEnable)
	{
		UpdateBuildingManagerValue();

		// 삭제, 드레그 추가해야함.
	}
}

void AGS_BuildManager::InitGrid()
{
	GridSize.X = FMath::Clamp(GridSize.X, 1, GridSize.X);
	GridSize.Y = FMath::Clamp(GridSize.Y, 1, GridSize.Y);
	
	CellsCount = GridSize.X * GridSize.Y;
	
	float UnrealCellSize = CellSize * 0.01f;
	StaticMeshCompo->SetRelativeScale3D(FVector(GridSize.X * UnrealCellSize, GridSize.Y * UnrealCellSize, 1.0f));
	
	if (nullptr != GridMaterial)
	{
		MIDGrid = UMaterialInstanceDynamic::Create(GridMaterial, this);

		if (nullptr != MIDGrid)
		{
			MIDGrid->SetScalarParameterValue("CellSize", CellSize);
			MIDGrid->SetVectorParameterValue("Color", GridColor);
			MIDGrid->SetScalarParameterValue("Opacity", GridOpacity);
			MIDGrid->SetTextureParameterValue("CellTexture", GridTexture);
		}
	}
}

void AGS_BuildManager::UpdateBuildingManagerValue()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			FHitResult HitResult;
			if (DEPC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Camera), bTraceComplex, HitResult))
			{
				LocationUnderCursorVisibility = HitResult.Location;
				ActorUnderCursor = HitResult.GetActor();
			}

			if (DEPC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), bTraceComplex, HitResult))
			{
				LocationUnderCursorCamera = HitResult.Location;
			}
		}
	}

	// 마우스 위치 변경 체크
	if (CellUnderCursor != LastCellUnderCursor)
	{
		LastCellUnderCursor = CellUnderCursor;
		bIsCellUnderCursorChanged = true;
	}
	else
	{
		bIsCellUnderCursorChanged = false;
	}
	CellUnderCursor =  GetCellFromWorldLocation(LocationUnderCursorCamera);
}

FVector2D AGS_BuildManager::GetCellCenter(FIntPoint CellIdx)
{
	return FVector2d(FMath::Sign(CellIdx.X) * FMath::Abs(CellIdx.X) * CellSize
		, FMath::Sign(CellIdx.Y) * FMath::Abs(CellIdx.Y) * CellSize);
}

FIntPoint AGS_BuildManager::GetCellFromWorldLocation(FVector InLocation)
{
	int X = FMath::RoundToInt(InLocation.X / CellSize);
	int Y = FMath::RoundToInt(InLocation.Y / CellSize);
	int NewX = FMath::Abs(X) * FMath::Sign(X);
	int NewY = FMath::Abs(Y) * FMath::Sign(Y);

	return FIntPoint(NewX, NewY);
}

FVector AGS_BuildManager::GetCellLocation(FIntPoint InCellUnderCurosr)
{
	FVector2d CellCenterLocation2D = GetCellCenter(InCellUnderCurosr);
	FVector CellCenterLocation = FVector(CellCenterLocation2D.X, CellCenterLocation2D.Y, 0.0f);

	FHitResult HitResult;
	FVector StartTraceLocation = FVector(CellCenterLocation.X, CellCenterLocation.Y, LocationUnderCursorCamera.Z + StartTraceHeight);
	FVector EndTraceLocation = FVector(CellCenterLocation.X, CellCenterLocation.Y, LocationUnderCursorCamera.Z + EndTraceHeight);
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTraceLocation, EndTraceLocation, ECC_Camera, CollisionParams))
	{
		CellCenterLocation.Z = HitResult.Location.Z;
	}
	
	return CellCenterLocation;
}

FVector2d AGS_BuildManager::GetCenterOfRectArea(FIntPoint InAreaCenterCell, FIntPoint AreaSize, float RotateDegree)
{
	FVector2d AreaCenter = 	GetCellCenter(InAreaCenterCell);

	FVector2D HalfCellOffset(0.f, 0.f);
	if (AreaSize.X % 2 == 0)
	{
		HalfCellOffset.X -= CellSize * 0.5f;
	}
	if (AreaSize.Y % 2 == 0)
	{
		HalfCellOffset.Y -= CellSize * 0.5f;
	}

	HalfCellOffset = HalfCellOffset.GetRotated(RotateDegree);
	AreaCenter += HalfCellOffset;
	
	return AreaCenter;
}

void AGS_BuildManager::GetCellsInRectArea(TArray<FIntPoint>& InIntPointArray, FIntPoint InCenterAreaCell, FIntPoint InAreaSize, float RotateDegree)
{
	FIntPoint StartCell = FIntPoint(InCenterAreaCell.X - FMath::FloorToInt((InAreaSize.X * 0.5f)), InCenterAreaCell.Y - FMath::FloorToInt((InAreaSize.Y * 0.5f)));

	for (int i = 0; i < InAreaSize.X; ++i)
	{
		int CurrentCellX = StartCell.X + i;
		for (int j = 0; j <InAreaSize.Y; ++j)
		{
			int CurrentCellY = StartCell.Y + j;
			FVector2D NewRotatePoint = FVector2D(CurrentCellX - InCenterAreaCell.X, CurrentCellY - InCenterAreaCell.Y).GetRotated(RotateDegree) + FVector2D(InCenterAreaCell.X, InCenterAreaCell.Y);
			NewRotatePoint.X = FMath::RoundToInt(NewRotatePoint.X);
			NewRotatePoint.Y = FMath::RoundToInt(NewRotatePoint.Y);
			int NewX = FMath::Abs(NewRotatePoint.X) * FMath::Sign(NewRotatePoint.X);
			int NewY = FMath::Abs(NewRotatePoint.Y) * FMath::Sign(NewRotatePoint.Y);
			InIntPointArray.Add(FIntPoint(NewX, NewY));
		}
	}
}

void AGS_BuildManager::SetOccupancyData(FIntPoint InCellPoint, EDEditorCellType InTargetType, EObjectType InObjectType, AActor* InActor, bool InIsRoom, bool InDeleteMode)
{
	FDEOccupancyData& CellInfo = OccupancyData.FindOrAdd(InCellPoint);

	if ((InIsRoom && InTargetType == EDEditorCellType::HorizontalPlaceable)
		|| InTargetType == EDEditorCellType::CeilingPlace
		|| InTargetType == EDEditorCellType::HorizontalPlaceable)
	{
		CellInfo.CeilingOccupancyData = InTargetType;
	}

	CellInfo.FloorOccupancyData = InTargetType;

	if (InDeleteMode)
	{
		switch (InObjectType)
		{
		case EObjectType::Monster:
			if (IsValid(CellInfo.FloorOccupancyActor))
			{
				if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(CellInfo.FloorOccupancyActor))
				{
					TargetMonster->DestroyAllWeapons();
					CellInfo.FloorOccupancyActor->Destroy();
				}
			}
			CellInfo.FloorOccupancyActor = nullptr;
			break;
		
		case EObjectType::Room:
			if (IsValid(CellInfo.FloorOccupancyActor))
			{
				CellInfo.FloorOccupancyActor->Destroy();
			}
			if (IsValid(CellInfo.CeilingOccupancyActor))
			{
				CellInfo.CeilingOccupancyActor->Destroy();
			}
			if (IsValid(CellInfo.RoomOccupancyActor))
			{
				CellInfo.RoomOccupancyActor->Destroy();
			}
			if (IsValid(CellInfo.WallAndDoorOccupancyActor))
			{
				CellInfo.WallAndDoorOccupancyActor->Destroy();
				TArray<FIntPoint> Coord = CellInfo.WallAndDoorOccupancyActor->GetComponentByClass<UPlaceInfoComponent>()->GetCellCoord();
				for (int i = 0; i < Coord.Num(); i++)
				{
					FDEOccupancyData& WallCellInfo = OccupancyData.FindChecked(Coord[i]);
					WallCellInfo.FloorOccupancyData = EDEditorCellType::WallAndDoorPlaceable;
					if (IsValid(WallCellInfo.FloorOccupancyActor))
					{
						if (WallCellInfo.FloorOccupancyActor->GetComponentByClass<UPlaceInfoComponent>()->GetTrapPlacement() == ETrapPlacement::Wall)
						{
							WallCellInfo.FloorOccupancyActor->Destroy();
						}
					}
				}
				CellInfo.FloorOccupancyData = InTargetType;
			}
			CellInfo.FloorOccupancyActor = nullptr;
			CellInfo.CeilingOccupancyActor = nullptr;
			CellInfo.WallAndDoorOccupancyActor = nullptr;
			CellInfo.RoomOccupancyActor = nullptr;
			break;
		
		case EObjectType::Trap:
			if (InTargetType == EDEditorCellType::CeilingPlace)
			{
				if (IsValid(CellInfo.CeilingOccupancyActor))
				{
					CellInfo.CeilingOccupancyActor->Destroy();
				}
				
				CellInfo.CeilingOccupancyActor = nullptr;
			}
			else
			{
				if (IsValid(CellInfo.FloorOccupancyActor))
				{
					CellInfo.FloorOccupancyActor->Destroy();
				}
				CellInfo.FloorOccupancyActor = nullptr;
			}
			break;
		
		case EObjectType::DoorAndWall:
			if (IsValid(CellInfo.FloorOccupancyActor))
			{
				CellInfo.FloorOccupancyActor->Destroy();
			}
			if (IsValid(CellInfo.WallAndDoorOccupancyActor))
			{
				CellInfo.WallAndDoorOccupancyActor->Destroy();
			}
			CellInfo.FloorOccupancyActor = nullptr;
			CellInfo.WallAndDoorOccupancyActor = nullptr;
			break;

		default:
			break;
		}
	}
	else
	{
		switch (InObjectType)
		{
		case EObjectType::Monster:
			CellInfo.FloorOccupancyActor = InActor;
			break;
		
		case EObjectType::Room:
			CellInfo.RoomOccupancyActor = InActor;
			break;
		
		case EObjectType::Trap:
			if (InTargetType == EDEditorCellType::CeilingPlace)
			{
				CellInfo.CeilingOccupancyActor = InActor;
			}
			else
			{
				CellInfo.FloorOccupancyActor = InActor;
			}
			break;
		
		case EObjectType::DoorAndWall:
			CellInfo.WallAndDoorOccupancyActor = InActor;
			break;

		default:
			break;
		}
	}
}

bool AGS_BuildManager::CheckOccupancyData(FIntPoint InCellPoint,EDEditorCellType InTargetType)
{
	EDEditorCellType FindOccupancyType;
	ConvertFindOccupancyData(InTargetType, FindOccupancyType);

	if (auto* CellData = OccupancyData.Find(InCellPoint))
	{
		const EDEditorCellType Current = 
			(InTargetType == EDEditorCellType::CeilingPlace)
				? CellData->CeilingOccupancyData
				: CellData->FloorOccupancyData;

		return Current != FindOccupancyType;
	}

	return InTargetType != EDEditorCellType::None;
}

void AGS_BuildManager::ConvertFindOccupancyData(EDEditorCellType InTargetType, EDEditorCellType& InOutFindType)
{
	switch (InTargetType)
	{
	case EDEditorCellType::None:
		InOutFindType = EDEditorCellType::None;
		break;
		
	case EDEditorCellType::WallPlace:
		InOutFindType = EDEditorCellType::VerticalPlaceable;
		break;

	case EDEditorCellType::FloorPlace:
		InOutFindType = EDEditorCellType::HorizontalPlaceable;
		break;
		
	case EDEditorCellType::CeilingPlace:
		InOutFindType = EDEditorCellType::HorizontalPlaceable;
		break;
		
	case EDEditorCellType::Wall:
	case EDEditorCellType::Door:
		InOutFindType = EDEditorCellType::WallAndDoorPlaceable;
		break;

	default:
		break;
	}
}

EDEditorCellType AGS_BuildManager::GetTargetCellType(EObjectType InObjectType, ETrapPlacement InTrapType)
{
	EObjectType ObjectType = InObjectType;
	if (ObjectType == EObjectType::Room)
	{
		return EDEditorCellType::None;
	}
	else if (ObjectType == EObjectType::Monster)
	{
		return EDEditorCellType::FloorPlace;
	}
	else if (ObjectType == EObjectType::Trap)
	{
		ETrapPlacement TrapType = InTrapType;
		if (TrapType == ETrapPlacement::Floor)
			return EDEditorCellType::FloorPlace;
		else if (TrapType == ETrapPlacement::Ceiling)
			return EDEditorCellType::CeilingPlace;
		else if (TrapType == ETrapPlacement::Wall)
			return EDEditorCellType::WallPlace;
	}
	else if (ObjectType == EObjectType::DoorAndWall)
	{
		// 나중에 벽과 문을 구분지어주어야 함.
		return EDEditorCellType::Door;
	}
	else
	{
		return EDEditorCellType::None;
	}
	return EDEditorCellType::None;
}

void AGS_BuildManager::EnableBuildingTool(FDataTableRowHandle* Data)
{
	// Disable Demolition Tool 추가해야함.
	
	ChangeObjectForPlacement(Data);

	if (bIsPlacementSelected)
	{
		bBuildToolEnabled = true;
		if (ActivePlacer)
		{
			// Deactivate Object Placer
			ActivePlacer->Destroy();
		}
		UpdateBuildingManagerValue();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(LocationUnderCursorCamera);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = PlaceableObjectData.Name;
		ActivePlacer = GetWorld()->SpawnActor<AGS_PlacerBase>(PlaceableObjectData.ObjectPlacerClass,SpawnTransform);
		ActivePlacer->ActiveObjectPlacer();

		if (DecalMaterialInstance)
		{
			ChangeDecalType(Data);
		}
	}
}

void AGS_BuildManager::ChangeObjectForPlacement(FDataTableRowHandle* Data)
{
	ObjectForPlacement = Data;
	if (ObjectForPlacement->RowName != FName("None"))
	{
		const FGS_PlaceableObjectsRow* RowPtr = ObjectForPlacement->GetRow<FGS_PlaceableObjectsRow>(ObjectForPlacement->RowName.ToString());
		if (RowPtr)
		{
			PlaceableObjectData = *RowPtr;
			bIsPlacementSelected = true;
		}
		else
		{
			bIsPlacementSelected = false;
		}
	}
	else
	{
		bIsPlacementSelected = false;
	}
}

void AGS_BuildManager::PressedLMB()
{
	bInteractStarted = true;
	SelectPlaceableObject();

	if (bIsPlacementSelected)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
			{
				FHitResult HitResult;
				if (DEPC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Camera), bTraceComplex, HitResult))
				{
					if (HitResult.bBlockingHit)
					{
						StartLocationUnderCursor = HitResult.Location;

						if (bBuildToolEnabled)
						{
							// BuildStart Event
						}
					}
				}
			}
		}
	}
}

void AGS_BuildManager::ReleasedLMB()
{
	if (bInteractStarted)
	{
		if (bBuildToolEnabled && bIsPlacementSelected)
		{
			if (ActivePlacer)
			{
				// Resources 체크해서 빌드해주도록 하면됨.
				ActivePlacer->BuildObject();
			}
			// Call Event Build Starteed
		}

		bInteractStarted = false;
	}
}

void AGS_BuildManager::SelectPlaceableObject()
{
	if (!bBuildToolEnabled)
	{
		if (PlaceableObjectUnderCursor)
		{
			if (SelectedPlacableObject)
			{
				if (PlaceableObjectUnderCursor != SelectedPlacableObject)
				{
					SelectedPlacableObject->SetObjectSelectedState(false);
				}
			}
			SelectedPlacableObject = PlaceableObjectUnderCursor;
			bPlaceableObjectSelected = true;
			SelectedPlacableObject->SetObjectSelectedState(true);
		}
		else
		{
			bPlaceableObjectSelected = false;
			if (SelectedPlacableObject)
			{
				SelectedPlacableObject->SetObjectSelectedState(false);
				SelectedPlacableObject = nullptr;
			}
		}
	}
}

void AGS_BuildManager::PressedRMB()
{
	if (bInteractStarted)
	{
		bInteractStarted = false;
	}

	if (bBuildToolEnabled && bIsPlacementSelected)
	{
		if (ActivePlacer)
		{
			ActivePlacer->Destroy();
			ActivePlacer = nullptr;
		}
		bBuildToolEnabled = false;
		bIsPlacementSelected = false;
		ClearDecalType();
	}
}

void AGS_BuildManager::ReleasedRMB()
{
}

void AGS_BuildManager::RotateProp()
{
	if (bBuildToolEnabled && bIsPlacementSelected)
	{
		if (ActivePlacer)
		{
			ActivePlacer->RotatePlacer();
		}
	}
}

void AGS_BuildManager::PressedDel()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AGS_DEController* DEPC = Cast<AGS_DEController>(PC))
		{
			FHitResult HitResult;
			if (DEPC->GetHitResultUnderCursor(ECC_GameTraceChannel6, bTraceComplex, HitResult))
			{
				AActor* DelActorUnderCursor = HitResult.GetActor();

				if (UPlaceInfoComponent* PlaceInfoComp = DelActorUnderCursor->GetComponentByClass<UPlaceInfoComponent>())
				{
					TArray<FIntPoint> CellCoords = PlaceInfoComp->GetCellCoord();
					EObjectType ObjectType = PlaceInfoComp->GetObjectType();
					ETrapPlacement TrapPlacement = PlaceInfoComp->GetTrapPlacement();
					
					EDEditorCellType TargetCellType = GetTargetCellType(ObjectType, TrapPlacement);
					EDEditorCellType ConvertTargetCellType;
					ConvertFindOccupancyData(TargetCellType, ConvertTargetCellType);

					const bool IsRoom = ObjectType == EObjectType::Room ? true : false;
					for (int i = 0; i < CellCoords.Num(); ++i)
					{
						SetOccupancyData(CellCoords[i], ConvertTargetCellType, ObjectType, nullptr, IsRoom, true);
					}
				}
			}
		}
	}
}

void AGS_BuildManager::ResetDungeonData()
{
	for (const auto& Pair : OccupancyData)
	{
		FIntPoint CellCoordinates = Pair.Key;
		const FDEOccupancyData& Data = Pair.Value;
		
		if (IsValid(Data.RoomOccupancyActor))
		{
			Data.RoomOccupancyActor->Destroy();
		}
		if (IsValid(Data.WallAndDoorOccupancyActor))
		{
			Data.WallAndDoorOccupancyActor->Destroy();
		}
		if (IsValid(Data.FloorOccupancyActor))
		{
			if (AGS_Monster* MonsterActor = Cast<AGS_Monster>(Data.FloorOccupancyActor))
			{
				MonsterActor->DestroyAllWeapons();
			}
			Data.FloorOccupancyActor->Destroy();
		}
		if (IsValid(Data.CeilingOccupancyActor))
		{
			Data.CeilingOccupancyActor->Destroy();
		}
	}
	
	OccupancyData.Empty();
}

void AGS_BuildManager::ChangeDecalType(FDataTableRowHandle* Data)
{
	if (nullptr == Data)
		return;
	
	ObjectForPlacement = Data;
	if (ObjectForPlacement->RowName != FName("None"))
	{
		const FGS_PlaceableObjectsRow* RowPtr = ObjectForPlacement->GetRow<FGS_PlaceableObjectsRow>(ObjectForPlacement->RowName.ToString());
		if (RowPtr)
		{
			PlaceableObjectData = *RowPtr;
			EObjectType SelectObjectType = PlaceableObjectData.ObjectType;
			ETrapPlacement SelectTrapType = PlaceableObjectData.TrapType;
			
			switch (SelectObjectType)
			{
				case EObjectType::Room:
					DecalMaterialInstance->SetScalarParameterValue("ShowType", 5);
					break;
				case EObjectType::Monster:
					DecalMaterialInstance->SetScalarParameterValue("ShowType", 6);
					break;
				
				case EObjectType::DoorAndWall:
					DecalMaterialInstance->SetScalarParameterValue("ShowType", 5);
					break;

				case EObjectType::Trap:
					switch (SelectTrapType)
					{
						case ETrapPlacement::Floor:
						case ETrapPlacement::Ceiling:
						DecalMaterialInstance->SetScalarParameterValue("ShowType", 6);
							break;
						
						case ETrapPlacement::Wall:
							DecalMaterialInstance->SetScalarParameterValue("ShowType", 5);
							break;

						default:
							DecalMaterialInstance->SetScalarParameterValue("ShowType", 7);
							break;
					}
					break;

				default:
					DecalMaterialInstance->SetScalarParameterValue("ShowType", 7);
					break;
			}
		}
	}
}

void AGS_BuildManager::ClearDecalType()
{
	if (DecalMaterialInstance)
	{
		DecalMaterialInstance->SetScalarParameterValue("ShowType", 7);
	}
}


void AGS_BuildManager::SaveDungeonData()
{
	// 1. 세이브 객체 인스턴스 생성 또는 로드
    UGS_DungeonEditorSaveGame* SaveGameObject;

    // UGameplayStatics를 사용해 지정된 슬롯에 세이브 파일이 이미 있는지 확인합니다.
    if (UGameplayStatics::DoesSaveGameExist(CurrentSaveSlotName, 0))
    {
        // 파일이 있다면, 파일로부터 객체를 불러옵니다.
        SaveGameObject = Cast<UGS_DungeonEditorSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveSlotName, 0));
        UE_LOG(LogTemp, Warning, TEXT("Existing SaveGame '%s' found. Loading."), *CurrentSaveSlotName);
    }
    else
    {
        // 파일이 없다면, 새로운 세이브 객체를 생성합니다.
        SaveGameObject = Cast<UGS_DungeonEditorSaveGame>(UGameplayStatics::CreateSaveGameObject(UGS_DungeonEditorSaveGame::StaticClass()));
        UE_LOG(LogTemp, Warning, TEXT("No SaveGame found. Creating new one for slot '%s'."), *CurrentSaveSlotName);
    }

    // 객체 생성/로드에 실패했다면 함수를 종료합니다. (항상 null 체크!)
    if (!IsValid(SaveGameObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create or load SaveGameObject."));
        return;
    }

    // 2. 월드의 액터 정보를 수집하여 세이브 객체에 채우기
    
    // 이전 데이터를 지우고 새로 채웁니다.
    SaveGameObject->ClearData();

    // TActorIterator를 사용해 월드에 있는 모든 AMyCube 액터를 순회합니다.
    // 'GetWorld()'는 현재 게임 월드에 대한 포인터를 반환합니다.
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* CurActor = *ActorItr;
    	
        // 액터가 유효하고, 파괴 중인 상태가 아닐 때만 저장합니다.
        if (IsValid(CurActor)
        	&& nullptr != CurActor->GetComponentByClass<UPlaceInfoComponent>())
        {
        	if (CurActor->ActorHasTag("Lobby"))
        	{
        		continue;
        	}
        	
            FDESaveData ObjectData;
        	ObjectData.SpawnActorClassPath = CurActor->GetClass()->GetPathName();
            //ObjectData.SpawnActorClass = CurActor->GetClass();
            ObjectData.SpawnTransform = CurActor->GetActorTransform();
            if (UPlaceInfoComponent* PlaceInfo = CurActor->FindComponentByClass<UPlaceInfoComponent>())
        	{
        		ObjectData.CellCoord = PlaceInfo->GetCellCoord();
        		ObjectData.ObjectType = PlaceInfo->GetObjectType();
        		ObjectData.TrapPlacement = PlaceInfo->GetTrapPlacement();
        	}

            SaveGameObject->AddSaveData(ObjectData);
        }
    }

	// 3. OccupancyData 저장
	for (const auto& Pair : OccupancyData)
	{
		FIntPoint CellCoordinates = Pair.Key;
		const FDEOccupancyData& Data = Pair.Value;

		SaveGameObject->FloorOccupancyData.FindOrAdd(CellCoordinates, Data.FloorOccupancyData);
		SaveGameObject->CeilingOccupancyData.FindOrAdd(CellCoordinates, Data.CeilingOccupancyData);
	}
	
    // 4. 파일에 최종 저장
    // 데이터가 채워진 세이브 객체를 슬롯에 저장합니다.
    bool bWasSaved = UGameplayStatics::SaveGameToSlot(SaveGameObject, CurrentSaveSlotName, 0);

    if (bWasSaved)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game Saved successfully to slot '%s'! Total actors saved: %d"), *CurrentSaveSlotName, SaveGameObject->GetDataCount());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot '%s'."), *CurrentSaveSlotName);
    }
}

void AGS_BuildManager::LoadDungeonData()
{
	// 1. 세이브 파일 존재 여부 확인
	if (!UGameplayStatics::DoesSaveGameExist(CurrentSaveSlotName, 0))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGame '%s' does not exist. Cannot load."), *CurrentSaveSlotName);
		return;
	}

	// 2. 파일로부터 데이터 로드 및 형변환(Cast)
	UGS_DungeonEditorSaveGame* LoadGameObject = Cast<UGS_DungeonEditorSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveSlotName, 0));

	if (!IsValid(LoadGameObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load SaveGame from slot '%s'. File might be corrupted."), *CurrentSaveSlotName);
		return;
	}
    
	UE_LOG(LogTemp, Warning, TEXT("SaveGame loaded successfully. Spawning actors..."));

	// 3.데이터 리셋
	ResetDungeonData();
	
	// 4. 로드한 데이터를 기반으로 월드에 액터 스폰 (Room 다음 Wall&Door 다음 나머지 프랍 순서)
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		TArray<FDESaveData> SortedObjectData = LoadGameObject->GetSaveDatas();

		auto GetSortPriority = [](EObjectType Type) -> int32 {
			switch (Type)
			{
			case EObjectType::Room: return 0;
			case EObjectType::DoorAndWall: return 1;
			default: return 2;
			}
		};

		SortedObjectData.Sort([&](const FDESaveData& A, const FDESaveData& B) {
		  return GetSortPriority(A.ObjectType) < GetSortPriority(B.ObjectType);
	  });
		
		for (const FDESaveData& ObjectData : SortedObjectData)
		{
			if (TSubclassOf<AActor> ActorClassToSpawn = LoadClass<AActor>(nullptr, *ObjectData.SpawnActorClassPath))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AActor* NewActor = World->SpawnActor<AActor>(ActorClassToSpawn, ObjectData.SpawnTransform, SpawnParams);

				bool Is_Room = false;
				if (NewActor)
				{
					if (AGS_RoomBase* NewRoom = Cast<AGS_RoomBase>(NewActor))
					{
						NewRoom->HideCeiling();
						NewRoom->UseDepthStencil();
						Is_Room = true;
					}
				}

				if (UPlaceInfoComponent* PlaceInfoCompo = NewActor->FindComponentByClass<UPlaceInfoComponent>())
				{
					PlaceInfoCompo->SetCellInfo(ObjectData.ObjectType, ObjectData.TrapPlacement, ObjectData.CellCoord);
					for (int i = 0; i < ObjectData.CellCoord.Num(); ++i)
					{
						EDEditorCellType TargetType = GetTargetCellType(ObjectData.ObjectType, ObjectData.TrapPlacement);
						SetOccupancyData(ObjectData.CellCoord[i], TargetType, ObjectData.ObjectType, NewActor, Is_Room);
						OccupancyData.FindOrAdd(ObjectData.CellCoord[i]).FloorOccupancyData = LoadGameObject->FloorOccupancyData.FindOrAdd(ObjectData.CellCoord[i]);
						OccupancyData.FindOrAdd(ObjectData.CellCoord[i]).CeilingOccupancyData = LoadGameObject->CeilingOccupancyData.FindOrAdd(ObjectData.CellCoord[i]);
					}
				}
			}

			// for (const auto& FloorData : LoadGameObject->FloorOccupancyData)
			// {
			// 	FIntPoint CellCoordinates = FloorData.Key;
			// 	const EDEditorCellType& Data = FloorData.Value;
			//
			// 	OccupancyData.FindOrAdd(CellCoordinates).FloorOccupancyData = Data;
			// }
			//
			// for (const auto& CeilingData : LoadGameObject->CeilingOccupancyData)
			// {
			// 	FIntPoint CellCoordinates = CeilingData.Key;
			// 	const EDEditorCellType& Data = CeilingData.Value;
			//
			// 	OccupancyData.FindOrAdd(CellCoordinates).CeilingOccupancyData = Data;
			// }
		}
	}
    
	UE_LOG(LogTemp, Warning, TEXT("Finished spawning %d actors from save file."), LoadGameObject->GetDataCount());
}
