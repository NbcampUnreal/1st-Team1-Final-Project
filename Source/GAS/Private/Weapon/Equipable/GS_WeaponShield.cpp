// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponShield.h"


// Sets default values
AGS_WeaponShield::AGS_WeaponShield()
{
	// server
	bReplicates = true;
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Set SKM
	ShieldMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShieldMeshComponent"));
	RootComponent = ShieldMeshComponent;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Shield_02/SKM_Shield_02_L.SKM_Shield_02_L"));
	
	if (MeshAsset.Succeeded())
	{
		ShieldMeshComponent->SetSkeletalMesh(MeshAsset.Object);
	}
}

void AGS_WeaponShield::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AGS_WeaponShield::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_WeaponShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

