// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTD_ClearRTSCommand.h"
#include "AI/GS_AIController.h"
#include "AI/RTS/RTSCommand.h"
#include "BehaviorTree/BlackboardComponent.h"

UGS_BTD_ClearRTSCommand::UGS_BTD_ClearRTSCommand()
{
	NodeName = TEXT("ClearRTS");
}

EBTNodeResult::Type UGS_BTD_ClearRTSCommand::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::None));
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
	
}
