// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_Seeker.h"
#include "GS_Ares.generated.h"

class AGS_SwordAuraProjectile;

UCLASS()
class GAS_API AGS_Ares : public AGS_Seeker
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Ares();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Projectile
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Projectile")
	TSubclassOf<AGS_SwordAuraProjectile> AresProjectileClass;

	virtual void OnComboAttack() override;

	virtual void ServerAttackMontage() override;

	virtual void MulticastPlayComboSection() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
