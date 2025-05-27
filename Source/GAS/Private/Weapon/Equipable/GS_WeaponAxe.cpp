// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponAxe.h"


// Sets default values
AGS_WeaponAxe::AGS_WeaponAxe()
{
	bReplicates = true;
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AxeMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AxeMeshComponent"));
	RootComponent = AxeMeshComponent;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Greataxe_01/SKM_Greataxe_01.SKM_Greataxe_01"));
	if (MeshAsset.Succeeded())
	{
		AxeMeshComponent->SetSkeletalMesh(MeshAsset.Object);
	}
}

// Called when the game starts or when spawned
void AGS_WeaponAxe::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_WeaponAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

