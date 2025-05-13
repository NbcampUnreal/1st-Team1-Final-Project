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
		UE_LOG(LogTemp, Warning, TEXT("[%s] No TargetActor in BB"), *GetName());
		return false;
	}
	
	float DistanceToTarget = FVector::Dist(ControllingPawn->GetActorLocation(), TargetActor->GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("[%s] Dist to target: %.1f"), *GetName(), DistanceToTarget);
	
	return DistanceToTarget <= AttackDistance;
}
