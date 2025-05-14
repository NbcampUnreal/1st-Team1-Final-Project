// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Merci.h"


// Sets default values
AGS_Merci::AGS_Merci()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_Merci::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_Merci::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGS_Merci::LeftClickPressed_Implementation()
{
	IAttackInterface::LeftClickPressed_Implementation();

	if(GetAttackMode() == ESeekerAttackMode::NonAttackMode)
	{
		SetAttackMode(ESeekerAttackMode::AimAttackMode);
	}
	SetAimState(true);
}

void AGS_Merci::LeftClickRelease_Implementation()
{
	IAttackInterface::LeftClickRelease_Implementation();

	SetAimState(false);
}

