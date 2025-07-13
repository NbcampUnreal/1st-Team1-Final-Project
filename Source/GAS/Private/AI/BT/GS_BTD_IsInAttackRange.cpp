// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTD_IsInAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/GS_AIController.h"

UGS_BTD_IsInAttackRange::UGS_BTD_IsInAttackRange()
{
	NodeName = TEXT("IsInAttackRange");
	AttackDistance = 50.0f;
}

bool UGS_BTD_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if(!ControllingPawn)
	{
		return false;
	}
	
	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AGS_AIController::TargetActorKey));
	if (!TargetActor)
	{
		return false;
	}
	
	float DistanceToTarget = FVector::Dist(ControllingPawn->GetActorLocation(), TargetActor->GetActorLocation());
	return DistanceToTarget <= AttackDistance;
}
