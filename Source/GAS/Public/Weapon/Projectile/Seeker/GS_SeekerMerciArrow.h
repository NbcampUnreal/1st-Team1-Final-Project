// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "GS_SeekerMerciArrow.generated.h"

class AGS_ArrowVisualActor;

UCLASS()
class GAS_API AGS_SeekerMerciArrow : public AGS_WeaponProjectile
{
	GENERATED_BODY()

protected:
	void StickWithVisualOnly(const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGS_ArrowVisualActor> VisualArrowClass;	
};
