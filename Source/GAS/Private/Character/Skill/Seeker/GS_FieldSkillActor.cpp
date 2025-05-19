// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/GS_FieldSkillActor.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"

// Sets default values
AGS_FieldSkillActor::AGS_FieldSkillActor()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("AreaShpere"));
	SphereComp->InitSphereRadius(Radius);
	SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComp;

	ParticleSystemComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VisualEffect"));
	ParticleSystemComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AGS_FieldSkillActor::BeginPlay()
{
	Super::BeginPlay();

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_FieldSkillActor::OnOverlapBegin);
	SphereComp->OnComponentEndOverlap.AddDynamic(this, &AGS_FieldSkillActor::OnOverlapEnd);

	ApplyFieldEffect();

	if (HasAuthority())
	{
		Multicast_DrawDebugSphere();
	}
	// 지속 시간 후 제거
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AGS_FieldSkillActor::DestroySelf, Duration);
	
}

void AGS_FieldSkillActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == Caster) return;

	if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(OtherActor))
	{
		ApplyFieldEffectToMonster(TargetMonster);
	}
	else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(OtherActor))
	{
		ApplyFieldEffectToGuardian(TargetGuardian);
	}
}

void AGS_FieldSkillActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == Caster) return;

	if (AGS_Monster* TargetMonster = Cast<AGS_Monster>(OtherActor))
	{
		RemoveFieldEffectFromMonster(TargetMonster);
	}
	else if (AGS_Guardian* TargetGuardian = Cast<AGS_Guardian>(OtherActor))
	{
		RemoveFieldEffectFromGuardian(TargetGuardian);
	}
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

void AGS_FieldSkillActor::RemoveFieldEffectFromMonster(AGS_Monster* Target)
{
}

void AGS_FieldSkillActor::ApplyFieldEffectToGuardian(AGS_Guardian* Target)
{
}

void AGS_FieldSkillActor::RemoveFieldEffectFromGuardian(AGS_Guardian* Target)
{
}

void AGS_FieldSkillActor::Multicast_DrawDebugSphere_Implementation()
{
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		Radius,
		16,
		FColor::Red,
		false,
		Duration
	);
}

