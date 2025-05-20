// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_WeaponEquipable.h"
#include "GS_WeaponBow.generated.h"

UCLASS()
class GAS_API AGS_WeaponBow : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_WeaponBow();

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Attack")
	USkeletalMeshComponent* BowMeshcomponent;

	UPROPERTY(VisibleAnywhere, Category = "Arrow")
	UChildActorComponent* Arrow;
};
