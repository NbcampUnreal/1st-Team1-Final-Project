// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GS_BTD_IsInAttackRange.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_BTD_IsInAttackRange : public UBTDecorator
{
	GENERATED_BODY()

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDistance;
	
};
