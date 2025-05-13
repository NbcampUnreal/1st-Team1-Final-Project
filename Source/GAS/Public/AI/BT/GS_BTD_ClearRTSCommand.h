// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GS_BTD_ClearRTSCommand.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_BTD_ClearRTSCommand : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UGS_BTD_ClearRTSCommand();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
