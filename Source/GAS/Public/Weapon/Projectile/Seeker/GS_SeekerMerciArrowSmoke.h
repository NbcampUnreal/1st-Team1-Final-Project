// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/Seeker/GS_SeekerMerciArrow.h"
#include "GS_SeekerMerciArrowSmoke.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_SeekerMerciArrowSmoke : public AGS_SeekerMerciArrow
{
	GENERATED_BODY()

public:
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult) override;
	
};
