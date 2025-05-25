// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "GS_BTS_SenseEnemy.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_BTS_SenseEnemy : public UBTService
{
	GENERATED_BODY()

public:
	UGS_BTS_SenseEnemy();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

};
