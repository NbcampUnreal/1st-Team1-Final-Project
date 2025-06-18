// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Ares.h"
#include "Character/Component/Seeker/GS_AresSkillInputHandlerComp.h"


// Sets default values
AGS_Ares::AGS_Ares()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CharacterType = ECharacterType::Ares;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_AresSkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
}

// Called when the game starts or when spawned
void AGS_Ares::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGS_Ares::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGS_Ares::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

