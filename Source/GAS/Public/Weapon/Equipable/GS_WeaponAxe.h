// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_WeaponEquipable.h"
#include "GS_WeaponAxe.generated.h"

UCLASS()
class GAS_API AGS_WeaponAxe : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_WeaponAxe();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* AxeMeshComponent;

	// 히트 사운드
	//UPROPERTY(EditDefaultsOnly, Category = "Sound")
	//UAkAudioEvent* HitSound;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
