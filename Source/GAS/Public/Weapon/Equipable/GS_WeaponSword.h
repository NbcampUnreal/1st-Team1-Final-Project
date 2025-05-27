// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Equipable/GS_WeaponMeleeBase.h"
#include "GS_WeaponSword.generated.h"

/**
 * 검 클래스
 */
UCLASS()
class GAS_API AGS_WeaponSword : public AGS_WeaponMeleeBase
{
	GENERATED_BODY()

public:
	AGS_WeaponSword();

protected:
	// 검 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* SwordMesh;
};