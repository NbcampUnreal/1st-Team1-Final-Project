// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTD_ClearRTSCommand.h"
#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UGS_BTD_ClearRTSCommand::UGS_BTD_ClearRTSCommand()
{
	NodeName = TEXT("ClearRTS");
}

EBTNodeResult::Type UGS_BTD_ClearRTSCommand::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsBool(AGS_AIController::bUseRTSKey, false);
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
	
}
