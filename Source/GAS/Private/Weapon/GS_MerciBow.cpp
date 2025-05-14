// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GS_MerciBow.h"


// Sets default values
AGS_MerciBow::AGS_MerciBow()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_MerciBow::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_MerciBow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

