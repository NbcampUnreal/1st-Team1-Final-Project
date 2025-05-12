// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Seeker.generated.h"

class UGS_SkillInputHandlerComp;

UCLASS()
class GAS_API AGS_Seeker : public AGS_Player
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGS_Seeker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Input")
	UGS_SkillInputHandlerComp* SkillInputHandlerComponent;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
