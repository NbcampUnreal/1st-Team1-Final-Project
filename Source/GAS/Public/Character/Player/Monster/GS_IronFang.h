// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "GS_IronFang.generated.h"

class AGS_WeaponSword;
/**
 * 
 */
UCLASS()
class GAS_API AGS_IronFang : public AGS_Monster
{
	GENERATED_BODY()

public:
	AGS_IronFang();

	UFUNCTION()
	AGS_WeaponSword* GetCurrentWeapon() const { return CurrentWeapon; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Weapon")
	TSubclassOf<AGS_WeaponSword> WeaponClass;	

	UPROPERTY()
	AGS_WeaponSword* CurrentWeapon;
};
