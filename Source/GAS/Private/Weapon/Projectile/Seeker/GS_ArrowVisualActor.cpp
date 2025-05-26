// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Net/UnrealNetwork.h"


AGS_ArrowVisualActor::AGS_ArrowVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);



	ArrowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetIsReplicated(true);
	SetRootComponent(ArrowMesh);

	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetSimulatePhysics(false);
}

void AGS_ArrowVisualActor::OnRep_SkeletalMesh()
{
		ArrowMesh->SetSkeletalMesh(CurrentMesh);
}

void AGS_ArrowVisualActor::SetArrowMesh(USkeletalMesh* Mesh)
{
	if (ArrowMesh && Mesh)
	{
		ArrowMesh->SetSkeletalMesh(Mesh);
		CurrentMesh = Mesh;
		UE_LOG(LogTemp, Warning, TEXT("Arrow Mesh Set!!!!!!!!!!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Arrow Mesh null or Mesh null"));
	}
}

void AGS_ArrowVisualActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_ArrowVisualActor, CurrentMesh);
}

