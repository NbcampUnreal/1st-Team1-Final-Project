// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GS_AIController.generated.h"

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
	static const FName TargetKey;

	UPROPERTY(VisibleAnywhere, Category = "AI Perception")
	class UAISenseConfig_Sight* SightConfig;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	UPROPERTY(EditAnywhere)
	UBehaviorTree* BTAsset;
	
	UPROPERTY(EditAnywhere)
	UBlackboardData* BBAsset;
};
