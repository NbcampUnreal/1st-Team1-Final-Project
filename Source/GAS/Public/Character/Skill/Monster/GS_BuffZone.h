// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_BuffZone.generated.h"

class AGS_Monster;
struct FGS_StatRow;

UCLASS()
class GAS_API AGS_BuffZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_BuffZone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* CollisionSphere;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float BuffRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float BuffDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackPowerBuff;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBuff;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackSpeedBuff;

private:
	UPROPERTY()
	TArray<AGS_Monster*> BuffedMonsters;
	
	FTimerHandle BuffRemovalTimer;
	
	void ApplyBuffInZone();
	void ApplyBuff(AGS_Monster* Monster);
	void RemoveAllBuffs();
	FGS_StatRow GetBuffStatRow() const;

};
