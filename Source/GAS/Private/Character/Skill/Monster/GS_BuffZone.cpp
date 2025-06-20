// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Monster/GS_BuffZone.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"


AGS_BuffZone::AGS_BuffZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
}


void AGS_BuffZone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			BuffRadius,
			32,
			FColor::Red,
			false,
			3.0f,
			0,
			3.0f
		);
		
		ApplyBuffInZone();
		
		GetWorld()->GetTimerManager().SetTimer(
			BuffRemovalTimer,
			[this](){RemoveAllBuffs();},
			BuffDuration,
			false
		);
		
		SetLifeSpan(BuffDuration + 0.5f);
	}
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
	UE_LOG(LogTemp, Log, TEXT("BuffZone: %d마리 몬스터"), OverlappingActors.Num());
	
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
	// TODO : 스탯 컴포넌트에서 증가하는거 실행
}

void AGS_BuffZone::RemoveAllBuffs()
{
	// TODO : 스탯 컴포넌트에서 다시 디폴트 되는거 실행

	GetWorld()->GetTimerManager().ClearTimer(BuffRemovalTimer);
}


