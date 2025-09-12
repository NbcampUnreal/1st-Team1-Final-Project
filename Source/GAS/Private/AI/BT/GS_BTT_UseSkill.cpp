// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTT_UseSkill.h"
#include "AI/GS_AIController.h"
#include "AI/RTS/RTSCommand.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Monster/GS_Monster.h"


UGS_BTT_UseSkill::UGS_BTT_UseSkill()
{
	NodeName = TEXT("UseSkill");
}

EBTNodeResult::Type UGS_BTT_UseSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AGS_AIController* AIController = Cast<AGS_AIController>(OwnerComp.GetAIOwner());
	if(!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AGS_Monster* Monster = Cast<AGS_Monster>(OwnerComp.GetAIOwner()->GetPawn());
	if(!Monster)
	{
		return EBTNodeResult::Failed;
	}
	
	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsEnum(AGS_AIController::CommandKey, static_cast<uint8>(ERTSCommand::None));
	}
	
	Monster->UseSkill();
	return EBTNodeResult::Succeeded;
}