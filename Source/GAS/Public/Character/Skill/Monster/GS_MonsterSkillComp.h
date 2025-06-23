// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_MonsterSkillComp.generated.h"


class UGS_MonsterSkillBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterSkillCooldownChanged, float, InCurrentCoolTime, float, InMaxCoolTime);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_MonsterSkillComp : public UActorComponent
{
	GENERATED_BODY()

public:    
	UGS_MonsterSkillComp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<UGS_MonsterSkillBase> MonsterSkill;

	UPROPERTY(BlueprintAssignable)
	FOnMonsterSkillCooldownChanged OnMonsterSkillCooldownChanged;

	UFUNCTION()
	void TryActivateSkill();

	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillCooldownRemaining() const { return CooldownRemaining; }
	
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillMaxCooltime() const { return SkillCooltime; }

	void SetCanUseSkill(bool InCanUseSkill);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:    
	UPROPERTY()
	FTimerHandle CooldownTimer;

	UPROPERTY() 
	FTimerHandle UIUpdateTimer;

	UPROPERTY(ReplicatedUsing=OnRep_CooldownRemaining)
	float CooldownRemaining;

	UPROPERTY(Replicated)
	bool bIsOnCooldown;

	UPROPERTY(Replicated)
	float SkillCooltime;

	UPROPERTY(Replicated)
	float SkillDamage;

	UPROPERTY(Replicated)
	bool bCanUseSkill;

	UFUNCTION()
	void OnRep_CooldownRemaining();
	
	void SetSkill();
	void StartCooldown(float CooldownTime);
	void CooldownFinished();
	void BroadcastCooldownUpdate();
};
