// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Equipable/GS_WeaponMeleeBase.h"
#include "GS_WeaponAxe.generated.h"

/**
 * 도끼 클래스
 */
UCLASS()
class GAS_API AGS_WeaponAxe : public AGS_WeaponMeleeBase
{
	GENERATED_BODY()

public:
	AGS_WeaponAxe();

protected:
	// 도끼 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* AxeMesh;
};