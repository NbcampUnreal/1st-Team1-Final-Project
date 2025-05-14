// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"


// Sets default values
AGS_Seeker::AGS_Seeker()
{
	Weapon = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponChildActor"));
	Weapon->SetupAttachment(GetMesh(), TEXT("Bow"));
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SkillInputHandlerComponent = CreateDefaultSubobject<UGS_SkillInputHandlerComp>(TEXT("SkillInputHandlerComp"));
}

// Called when the game starts or when spawned
void AGS_Seeker::BeginPlay()
{
	Super::BeginPlay();

	if (SkillInputHandlerComponent)
	{
		SkillInputHandlerComponent->SetupEnhancedInput(InputComponent); // 여기에서 시도
	}
}

void AGS_Seeker::LeftClickPressed()
{
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

	if (SkillInputHandlerComponent)
	{
		SkillInputHandlerComponent->SetupEnhancedInput(PlayerInputComponent);
	}
}

void AGS_Seeker::SetAttackMode(ESeekerAttackMode Mode)
{
	AttackMode = Mode;
}

ESeekerAttackMode AGS_Seeker::GetAttackMode()
{
	return AttackMode;
}

void AGS_Seeker::SetAimState(bool IsAim)
{
	SeekerState.IsAim = IsAim;
}

bool AGS_Seeker::GetAimState()
{
	return SeekerState.IsAim;
}
