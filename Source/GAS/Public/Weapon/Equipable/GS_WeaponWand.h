// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Equipable/GS_WeaponEquipable.h"
#include "GS_WeaponWand.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_WeaponWand : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	AGS_WeaponWand();

	UPROPERTY(VisibleAnywhere, Category = "Attack")
	USkeletalMeshComponent* Mesh;

	USkeletalMeshComponent* GetWeaponMesh() const { return Mesh; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class AGS_Character* OwnerChar;
};
