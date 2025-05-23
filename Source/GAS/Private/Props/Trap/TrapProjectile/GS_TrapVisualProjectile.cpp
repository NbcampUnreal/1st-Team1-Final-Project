#include "Props/Trap/TrapProjectile/GS_TrapVisualProjectile.h"


AGS_TrapVisualProjectile::AGS_TrapVisualProjectile()
{	
	bReplicates = true;
	SetReplicateMovement(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	SetRootComponent(ProjectileMesh);

	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMeshObj(TEXT("/Game/Props/Trap/WallTrap/ArrowTrap/Mesh/SM_Arrow_old.SM_Arrow_old"));
	if (ArrowMeshObj.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(ArrowMeshObj.Object);
	}
}


void AGS_TrapVisualProjectile::SetProjectileMesh(UStaticMesh* Mesh)
{
	if (ProjectileMesh && Mesh)
	{
		ProjectileMesh->SetStaticMesh(Mesh);
	}
}
