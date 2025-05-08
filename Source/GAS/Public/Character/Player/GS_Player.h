// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "GS_Player.generated.h"

UCLASS()
class GAS_API AGS_Player : public AGS_Character
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Player();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
