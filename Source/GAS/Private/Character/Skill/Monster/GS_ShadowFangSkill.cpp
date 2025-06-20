// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Monster/GS_ShadowFangSkill.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Skill/Monster/GS_BuffZone.h"


UGS_ShadowFangSkill::UGS_ShadowFangSkill()
{
	BuffZoneClass = AGS_BuffZone::StaticClass();

	Cooltime = 20.0f; 
	Damage = 0.0f;
}

void UGS_ShadowFangSkill::ActiveSkill()
{
	Super::ActiveSkill();

	SpawnBuffZone();
}

void UGS_ShadowFangSkill::SpawnBuffZone()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	FVector SpawnLocation = OwnerCharacter->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.Owner = OwnerCharacter;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<AGS_BuffZone>(BuffZoneClass, SpawnLocation, SpawnRotation, Params);
}
