// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "BehaviorTree/BehaviorTree.h"       
#include "BehaviorTree/BlackboardData.h"
#include "AkGameplayStatics.h"
#include "GS_Monster.generated.h"

class UGS_MonsterAnimInstance;
/**
 * 
 */
UCLASS()
class GAS_API AGS_Monster : public AGS_Character
{
	GENERATED_BODY()

public:
	AGS_Monster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AvoidanceRadius = 200.0f;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BTAsset;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	UBlackboardData* BBAsset;

	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* ClickSoundEvent;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAkAudioEvent* MoveSoundEvent;
	
	void SetSelected(bool bIsSelected);

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void Attack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage();
	
protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	UPROPERTY()
	TObjectPtr<UGS_MonsterAnimInstance> MonsterAnim;
	
	UPROPERTY(VisibleAnywhere)
	UDecalComponent* SelectionDecal;

	UPROPERTY(VisibleAnywhere)
	UAkComponent* AkComponent;
	
};

