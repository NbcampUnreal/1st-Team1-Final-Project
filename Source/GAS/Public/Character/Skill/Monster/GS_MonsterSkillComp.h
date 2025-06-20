// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_MonsterSkillComp.generated.h"


class UGS_MonsterSkillBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_MonsterSkillComp : public UActorComponent
{
	GENERATED_BODY()

public:    
	UGS_MonsterSkillComp();

	UFUNCTION()
	void TryActivateSkill();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<UGS_MonsterSkillBase> MonsterSkill;

private:    
	UPROPERTY()
	FTimerHandle CooldownTimer; 
    
	UPROPERTY(Replicated)
	bool bIsOnCooldown;

	UPROPERTY(Replicated)
	float CooldownRemaining;

	UPROPERTY(Replicated)
	float SkillCooltime;

	UPROPERTY(Replicated)
	float SkillDamage;
	
	void SetSkill();
	void StartCooldown(float CooldownTime);
	void UpdateCooldownRemaining();
};
