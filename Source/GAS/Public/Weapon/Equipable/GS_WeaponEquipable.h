// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/GS_Weapon.h"
#include "GS_WeaponEquipable.generated.h"

class AGS_Seeker;

UCLASS()
class GAS_API AGS_WeaponEquipable : public AGS_Weapon
{
	GENERATED_BODY()
public:
	AGS_WeaponEquipable();

	virtual void SetOwningCharacter(AGS_Seeker* Character);

protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY()
	AGS_Seeker* OwnerCharacter;
};
