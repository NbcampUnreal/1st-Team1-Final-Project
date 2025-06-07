#include "Props/Trap/TrapProjectile/GS_TrapVisualProjectile.h"


AGS_TrapVisualProjectile::AGS_TrapVisualProjectile()
{	
	bReplicates = true;
	SetReplicateMovement(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	SetRootComponent(ProjectileMesh);

	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetSimulatePhysics(false);
	ProjectileMesh->SetRelativeScale3D(FVector(0.5f));

	//박히는 화살 에셋 여기서 교체
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshObj(TEXT("/Game/Props/Trap/WallTrap/ArrowTrap/Mesh/Final/Arrow_Head.Arrow_Head"));
	if (ArrowMeshObj.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(ArrowMeshObj.Object);
		
	}
}


void AGS_TrapVisualProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(5.0f);
}


void AGS_TrapVisualProjectile::SetProjectileMesh(UStaticMesh* Mesh)
{
	if (ProjectileMesh && Mesh)
	{
		ProjectileMesh->SetStaticMesh(Mesh);
	}
}
