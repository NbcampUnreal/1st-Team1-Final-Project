// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "GS_NeedleFang.generated.h"

/**
 * 
 */
class AGS_NeedleFangProjectile;

UCLASS()
class GAS_API AGS_NeedleFang : public AGS_Monster
{
	GENERATED_BODY()

public:
	AGS_NeedleFang();
	
	UFUNCTION(Server, Reliable)
	void Server_SpawnProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AGS_NeedleFangProjectile> ProjectileClass;
};
