// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Monster/GS_BuffZone.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"


AGS_BuffZone::AGS_BuffZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);

	BuffRadius = 1000.0f;
	BuffDuration = 10.0f;
	AttackPowerBuff = 30.0f;
	DefenseBuff = 30.0f;
	AttackSpeedBuff = 0.5f;
}


void AGS_BuffZone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ApplyBuffInZone();
		
		TWeakObjectPtr<AGS_BuffZone> WeakThis = this;
		GetWorld()->GetTimerManager().SetTimer(
			BuffRemovalTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->RemoveAllBuffs();
				}
			},
			BuffDuration,
			false
		);
		
		SetLifeSpan(BuffDuration + 0.5f);
	}
}

void AGS_BuffZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BuffRemovalTimer);
	}

	RemoveAllBuffs();
	BuffedMonsters.Empty();
	
	Super::EndPlay(EndPlayReason);
}

void AGS_BuffZone::ApplyBuffInZone()
{
	TArray<AActor*> OverlappingActors;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		GetActorLocation(),
		BuffRadius,
		{},
		AGS_Monster::StaticClass(),
		{GetOwner()},
		OverlappingActors
	);
	
	for (AActor* Actor : OverlappingActors)
	{
		if (AGS_Monster* Monster = Cast<AGS_Monster>(Actor))
		{
			ApplyBuff(Monster);
		}
	}
}

void AGS_BuffZone::ApplyBuff(AGS_Monster* Monster)
{
	UGS_StatComp* StatComp = Monster->GetStatComp();
	if (!StatComp)
	{
		return;
	}
	
	FGS_StatRow BuffStats = GetBuffStatRow();
	StatComp->ChangeStat(BuffStats);
	
	BuffedMonsters.Add(Monster);
}

void AGS_BuffZone::RemoveAllBuffs()
{
	FGS_StatRow BuffStats = GetBuffStatRow();
	
	for (int32 i = BuffedMonsters.Num() - 1; i >= 0; i--)
	{
		AGS_Monster* Monster = BuffedMonsters[i];
        
		if (IsValid(Monster))
		{
			UGS_StatComp* StatComp = Monster->GetStatComp();
			if (IsValid(StatComp))
			{
				StatComp->ResetStat(BuffStats);
			}
		}
		
		BuffedMonsters.RemoveAt(i);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BuffRemovalTimer);
	}
}

FGS_StatRow AGS_BuffZone::GetBuffStatRow() const
{
	FGS_StatRow Stat;
	Stat.ATK = AttackPowerBuff;
	Stat.DEF = DefenseBuff;
	Stat.ATS = AttackSpeedBuff;
    
	return Stat;
}  