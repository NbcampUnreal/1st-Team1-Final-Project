// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "GS_BTS_Cooldown.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_BTS_Cooldown : public UBTService
{
	GENERATED_BODY()

public:
	UGS_BTS_Cooldown();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
