// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Equipable/GS_WeaponEquipable.h"
#include "GS_WeaponSword.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_WeaponSword : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	AGS_WeaponSword();

	UFUNCTION()
	void EnableHit();
	
	UFUNCTION()
	void DisableHit();
	
	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Attack")
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Attack")
	class UBoxComponent* HitBox;

	UPROPERTY()
	class AGS_Character* OwnerChar;
};
