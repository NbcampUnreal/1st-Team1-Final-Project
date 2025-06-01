// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GS_AIController.generated.h"

struct FAIStimulus;
class AGS_Character;
/**
 * 
 */
UCLASS()
class GAS_API AGS_AIController : public AAIController
{
	GENERATED_BODY()

public:
	AGS_AIController();

	static const FName HomePosKey;
	static const FName MoveLocationKey;
	static const FName TargetActorKey;
	static const FName CommandKey;
	static const FName TargetLockedKey;
	static const FName CanAttackKey;
	static const FName LastAttackTimeKey;

	UPROPERTY(VisibleAnywhere, Category = "AI Perception")
	class UAISenseConfig_Sight* SightConfig;
	
	void LockTarget(AGS_Character* TargetCharacter);
	void UnlockTarget();

	void EnterConfuseState();
	void ExitConfuseState();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	UPROPERTY()
	UBehaviorTree* BTAsset;

	UPROPERTY()
	UBlackboardData* BBAsset;

	UPROPERTY()
	TWeakObjectPtr<AActor> PrevTargetActor;
};
