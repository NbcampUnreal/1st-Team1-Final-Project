// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RTS/GS_RTSCamera.h"

// Sets default values
AGS_RTSCamera::AGS_RTSCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGS_RTSCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_RTSCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

