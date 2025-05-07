// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/GS_Character.h"


// Sets default values
AGS_Character::AGS_Character()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

