// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTT_StopMovement.h"
#include "AI/GS_AIController.h"

UGS_BTT_StopMovement::UGS_BTT_StopMovement()
{
	NodeName = TEXT("Stop Movement");
}

EBTNodeResult::Type UGS_BTT_StopMovement::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AGS_AIController* AIController = Cast<AGS_AIController>(OwnerComp.GetAIOwner()))
	{
		AIController->StopMovement();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
