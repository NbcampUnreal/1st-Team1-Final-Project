// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTD_IsInAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/GS_AIController.h"

bool UGS_BTD_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if(!ControllingPawn)
	{
		return false;
	}
	
	const FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(AGS_AIController::TargetKey);
	const float DistanceToTarget = FVector::Dist(ControllingPawn->GetActorLocation(), TargetLocation);
	
	return DistanceToTarget <= AttackDistance;
}
