// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/GS_Player.h"


// Sets default values
AGS_Player::AGS_Player()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_Player::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGS_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

