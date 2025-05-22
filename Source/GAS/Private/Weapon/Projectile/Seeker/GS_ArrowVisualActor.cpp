// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"


AGS_ArrowVisualActor::AGS_ArrowVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ArrowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArrowMesh"));
	SetRootComponent(ArrowMesh);

	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetSimulatePhysics(false);
}

void AGS_ArrowVisualActor::SetArrowMesh(USkeletalMesh* Mesh)
{
	if (ArrowMesh && Mesh)
	{
		ArrowMesh->SetSkeletalMesh(Mesh);
	}
}

