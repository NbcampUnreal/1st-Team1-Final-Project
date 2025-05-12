// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GS_BTT_TurnToTarget.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_BTT_TurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UGS_BTT_TurnToTarget();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category = "Turn")
	float RotationSpeed;

	UPROPERTY(EditAnywhere, Category = "Turn")
	float AcceptanceAngle;
};
