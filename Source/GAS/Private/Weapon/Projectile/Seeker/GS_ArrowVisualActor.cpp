// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_ArrowVisualActor.h"
#include "Net/UnrealNetwork.h"


AGS_ArrowVisualActor::AGS_ArrowVisualActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SetReplicateMovement(true);

	ArrowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetIsReplicated(true);
	ArrowMesh->SetupAttachment(Root);

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
	}
}

void AGS_ArrowVisualActor::SetAttachedTargetActor(AActor* Target)
{
	AttachedTargetActor = Target;
	if (AttachedTargetActor)
	{
		AttachedTargetActor->OnDestroyed.AddDynamic(this, &AGS_ArrowVisualActor::OnAttachedTargetDestroyed);
	}
}

void AGS_ArrowVisualActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_ArrowVisualActor, CurrentMesh);
}

void AGS_ArrowVisualActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	if (ElapsedTime >= LifeTime)
	{
		Destroy(); // 또는 Object Pool로 반환
	}
}

