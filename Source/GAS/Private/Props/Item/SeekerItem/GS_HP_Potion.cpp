// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Item/SeekerItem/GS_HP_Potion.h"
#include "Props/Item/E_ItemType.h"


// Sets default values
AGS_HP_Potion::AGS_HP_Potion()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetSimulatePhysics(false);
}

// Called when the game starts or when spawned
void AGS_HP_Potion::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGS_HP_Potion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGS_HP_Potion::DropFromSocket()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (MeshComp)
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetEnableGravity(true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		const FVector LaunchImpulseVector = (-GetActorForwardVector() * 250.f) + (-FVector::UpVector * 120.f);
		MeshComp->AddImpulse(LaunchImpulseVector, NAME_None, true);

		const FVector RandomTorqueVector = FVector(
				FMath::FRandRange(-100.f, 100.f),
				FMath::FRandRange(-100.f, 100.f),
				FMath::FRandRange(-100.f, 100.f)
				);
		MeshComp->AddAngularImpulseInDegrees(RandomTorqueVector, NAME_None, true);
	}

	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AGS_Item::ItemDestroy, 5.f, false);
}
