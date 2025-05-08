// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Seeker.h"


// Sets default values
AGS_Seeker::AGS_Seeker()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_Seeker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_Seeker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGS_Seeker::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

