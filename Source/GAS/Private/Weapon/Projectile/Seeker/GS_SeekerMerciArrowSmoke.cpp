// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrowSmoke.h"
#include "Character/Skill/Seeker/Merci/GS_SmokeFieldSkill.h"

void AGS_SeekerMerciArrowSmoke::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ETargetType TargetType = DetermineTargetType(OtherActor);

	if (TargetType == ETargetType::Guardian || TargetType == ETargetType::DungeonMonster)
	{
		SpawnSmokeArea(SweepResult.ImpactPoint);
		StickWithVisualOnly(SweepResult);
	}
	else if (TargetType == ETargetType::Etc)
	{
		SpawnSmokeArea(SweepResult.ImpactPoint);
	}
}

void AGS_SeekerMerciArrowSmoke::SpawnSmokeArea(FVector SpawnLocation)
{
	if (SmokeAreaClass)
	{
		GetWorld()->SpawnActor<AGS_SmokeFieldSkill>(SmokeAreaClass, SpawnLocation, FRotator::ZeroRotator);
	}
}
