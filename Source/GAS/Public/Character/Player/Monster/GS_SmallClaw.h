// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "GS_SmallClaw.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_SmallClaw : public AGS_Monster
{
	GENERATED_BODY()

public:
	AGS_SmallClaw();
	
	UPROPERTY(VisibleAnywhere, Category = "Attack")
	class UBoxComponent* BiteCollision;

	UFUNCTION()
	void SetBiteCollision(bool bEnable);

	UFUNCTION()
	void OnAttackBiteboxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
protected:
	virtual void BeginPlay() override;
}; 