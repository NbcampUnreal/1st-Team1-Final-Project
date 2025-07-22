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
	AGS_AIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const FName HomePosKey;
	static const FName MoveLocationKey;
	static const FName TargetActorKey;
	static const FName CommandKey;
	static const FName TargetLockedKey;
	static const FName DebuffLockedKey;
	static const FName CanAttackKey;
	static const FName LastAttackTimeKey;

	UPROPERTY(VisibleAnywhere, Category = "AI Perception")
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void SetNewTarget(AActor* NewTarget);
	
	UFUNCTION()
	void OnTargetDied();
	
	void ClearCurrentTarget();
	
	void LockTarget(AGS_Character* TargetCharacter);
	void UnlockTarget();

	void EnterConfuseState();
	void ExitConfuseState();

	void SetNearPlayer(bool bNear);
	void SetRtsControl(bool bActive); 

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	UPROPERTY()
	UBehaviorTree* BTAsset;

	UPROPERTY()
	UBlackboardData* BBAsset;

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> PrevTargetActor;

	bool bNearPlayer;
	bool bRtsControl;
	bool bAIActive;

	UPROPERTY()
	TWeakObjectPtr<AGS_Character> TargetCharacter;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void UpdateAIState();
	
};
