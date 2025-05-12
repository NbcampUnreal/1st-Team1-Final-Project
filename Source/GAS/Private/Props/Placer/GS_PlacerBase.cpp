#include "Props/Placer/GS_PlacerBase.h"

AGS_PlacerBase::AGS_PlacerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshCompo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshCompo->SetupAttachment(RootComponent);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		TEXT("StaticMesh'/Game/DungeonEditor/Meshes/SM_Plane_100x100.SM_Plane_100x100'"));
	if (MeshFinder.Succeeded())
	{
		UStaticMesh* PlaneMesh = MeshFinder.Object;
		if (nullptr != PlaneMesh)
		{
			StaticMeshCompo->SetStaticMesh(PlaneMesh);
		}
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlaceAcceptedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Place_Indicators/MI_Place_Accepted.MI_Place_Accepted'"));
	if (PlaceAcceptedMatFinder.Succeeded())
	{
		PlaceAcceptedMaterial = PlaceAcceptedMatFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlaceRejectedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Place_Indicators/MI_Place_Rejected.MI_Place_Rejected'"));
	if (PlaceRejectedMatFinder.Succeeded())
	{
		 PlaceRejectedMaterial = PlaceRejectedMatFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BuildAcceptedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Building_Indicators/MI_Building_Accepted.MI_Building_Accepted'"));
	if (BuildAcceptedMatFinder.Succeeded())
	{
		BuildAcceptedMaterial = BuildAcceptedMatFinder.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BuildRejectedMatFinder(
		TEXT("MaterialInstanceConstant'/Game/DungeonEditor/Materials/Building_Indicators/MI_Building_Rejected.MI_Building_Rejected'"));
	if (BuildRejectedMatFinder.Succeeded())
	{
		BuildRejectedMaterial = BuildRejectedMatFinder.Object;
	}
}

void AGS_PlacerBase::BeginPlay()
{
	Super::BeginPlay();

	SetupObjectPlacer();
}

void AGS_PlacerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGS_PlacerBase::SetupObjectPlacer()
{
	// 데이터 테이블에서 찾아서 세팅해야함.

}