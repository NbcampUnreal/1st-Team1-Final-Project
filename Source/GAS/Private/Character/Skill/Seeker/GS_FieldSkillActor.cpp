// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"

// Sets default values
AGS_FieldSkillActor::AGS_FieldSkillActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("AreaShpere"));
	SphereComp->InitSphereRadius(200.0f);
	SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComp;

	ParticleSystemComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VisualEffect"));
	ParticleSystemComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AGS_FieldSkillActor::BeginPlay()
{
	Super::BeginPlay();

	// 지속 시간 후 제거
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AGS_FieldSkillActor::DestroySelf, Duration);
	
}

void AGS_FieldSkillActor::ApplyFieldEffect()
{
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (AGS_Character* Enemy = Cast<AGS_Character>(Actor))
		{
			if (Enemy && Enemy != Caster)
			{
				if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(Enemy))
				{
					ApplyFieldEffectToMonster(TargetMonster);
				}
				else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(Enemy))
				{
					ApplyFieldEffectToGuardian(TargetGuardian);
				}
			}
		}
		
	}
}

void AGS_FieldSkillActor::DestroySelf()
{
	Destroy();
}

void AGS_FieldSkillActor::ApplyFieldEffectToMonster(AGS_Monster* Target)
{
}

void AGS_FieldSkillActor::ApplyFieldEffectToGuardian(AGS_Guardian* Target)
{
}

