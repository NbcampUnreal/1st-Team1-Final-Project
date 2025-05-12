#include "DungeonEditor/GS_BuildManager.h"
#include "Components/BillboardComponent.h"

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
		GridTexture = TexFinder.Object;  // GridTexture에 할당
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