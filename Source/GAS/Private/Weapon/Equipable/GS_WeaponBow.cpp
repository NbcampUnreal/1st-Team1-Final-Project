// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponBow.h"


// Sets default values
AGS_WeaponBow::AGS_WeaponBow()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BowMeshcomponent = CreateDefaultSubobject<USkeletalMeshComponent>("BowMesh");
	RootComponent = BowMeshcomponent;

	Arrow = CreateDefaultSubobject<UChildActorComponent>("Arrow");
	
	Arrow->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AGS_WeaponBow::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_WeaponBow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

