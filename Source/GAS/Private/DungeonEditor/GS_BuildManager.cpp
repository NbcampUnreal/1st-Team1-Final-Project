#include "DungeonEditor/GS_BuildManager.h"
#include "Components/BillboardComponent.h"
#include "DungeonEditor/GS_DEController.h"
#include "DungeonEditor/Data/GS_PlaceableObjectsRow.h"
#include "Props/Placer/GS_PlacerBase.h"

AGS_BuildManager::AGS_BuildManager()
{
	PrimaryActorTick.bCanEverTick = true;

	BillboardCompo = CreateDefaultSubobject<UBillboardComponent>("BillBoard");
	SetRootComponent(BillboardCompo);
	StaticMeshCompo = CreateDefaultSubobject<UStaticMeshComponent>("GridMesh");
	StaticMeshCompo->SetupAttachment(BillboardCompo);

	// static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
	// 	TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	// if (MeshFinder.Succeeded())
	// {
	// 	UStaticMesh* PlaneMesh = MeshFinder.Object;
	// 	if (nullptr != PlaneMesh)
	// 	{
	// 		StaticMeshCompo->SetStaticMesh(PlaneMesh);
	// 	}
	// }
	
	bTraceComplex = false;
	StartTraceHeight = 3000;
	EndTraceHeight = -3000;
	GridSize = {32, 32};
	CellSize = 100.0f;
	GridColor = {0.0f, 0.853f, 1.0f, 1.0f};
	GridOpacity = 50.0f;

	bIsCellUnderCursorChanged = false;
	
	InitGrid();
}

#if WITH_EDITOR
void AGS_BuildManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (nullptr != MIDGrid)
	{
		StaticMeshCompo->SetMaterial(0, MIDGrid);
	}
}
#endif

void AGS_BuildManager::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr != MIDGrid)
	{
		StaticMeshCompo->SetMaterial(0, MIDGrid);
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

FVector2d AGS_BuildManager::GetCenterOfRectArea(FIntPoint InAreaCenterCell, FIntPoint AreaSize)
{
	FVector2d AreaCenter = 	GetCellCenter(InAreaCenterCell);
	if (AreaSize.X % 2 == 0)
	{
		AreaCenter.X -= CellSize * 0.5f;
	}
	if (AreaSize.Y % 2 == 0)
	{
		AreaCenter.Y -= CellSize * 0.5f;
	}

	return AreaCenter;
}

void AGS_BuildManager::GetCellsInRectArea(TArray<FIntPoint>& InIntPointArray, FIntPoint InCenterAreaCell, FIntPoint InAreaSize)
{
	FIntPoint StartCell = FIntPoint(InCenterAreaCell.X - FMath::FloorToInt((InAreaSize.X * 0.5f)), InCenterAreaCell.Y - FMath::FloorToInt((InAreaSize.Y * 0.5f)));

	for (int i = 0; i < InAreaSize.X; ++i)
	{
		int CurrentCellX = StartCell.X + i;
		for (int j = 0; j <InAreaSize.Y; ++j)
		{
			int CurrentCellY = StartCell.Y + j;
			InIntPointArray.Add(FIntPoint(CurrentCellX, CurrentCellY));
		}
	}
}

void AGS_BuildManager::SetOccupancyData(FIntPoint InCellPoint, bool InbOccipied)
{
	if (InbOccipied)
	{
		OccupancyData.FindOrAdd(InCellPoint) = 1;
	}
	else
	{
		if (OccupancyData.Find(InCellPoint))
		{
			if (OccupancyData[InCellPoint] < 2)
			{
				OccupancyData.Remove(InCellPoint);
			}
			else
			{
				OccupancyData[InCellPoint]--;
			}
		}
	}
}

bool AGS_BuildManager::CheckOccupancyData(FIntPoint InCellPoint)
{
	if (OccupancyData.Find(InCellPoint))
	{
		return true;
	}
	else
	{
		return false;
	}
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
		if (PlaceableobjectUnderCursor)
		{
			if (SelectedPlacableObject)
			{
				if (PlaceableobjectUnderCursor != SelectedPlacableObject)
				{
					SelectedPlacableObject->SetObjectSelectedState(false);
				}
			}
			SelectedPlacableObject = PlaceableobjectUnderCursor;
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
