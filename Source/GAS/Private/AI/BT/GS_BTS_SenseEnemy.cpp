// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTS_SenseEnemy.h"
#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UGS_BTS_SenseEnemy::UGS_BTS_SenseEnemy()
{
	NodeName = TEXT("Sense Enemy");
	bNotifyTick = true;
}

void UGS_BTS_SenseEnemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AGS_AIController* AIController = Cast<AGS_AIController>(OwnerComp.GetAIOwner());
	if(!AIController)
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard->GetValueAsBool(AGS_AIController::DebuffLockedKey))
	{
		return;
	}
	
	if (Blackboard->GetValueAsBool(AGS_AIController::TargetLockedKey))
	{
		return;
	}

	TArray<AActor*> Targets;
	AIController->PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Targets);
	if (Targets.IsEmpty()) 
	{
		if (Blackboard->GetValueAsObject(AGS_AIController::TargetActorKey) != nullptr)
		{
			AIController->ClearCurrentTarget();
		}
		
		return;
	}

	// 가장 가까운 적 찾기
	APawn* ControlledPawn = Cast<APawn>(AIController->GetPawn());
	float ClosestDist = TNumericLimits<float>::Max();
	AActor* NearestTarget = nullptr;

	for (AActor* Target : Targets)
	{
		const float Dist = FVector::DistSquared(ControlledPawn->GetActorLocation(),	Target->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			NearestTarget = Target;
		}
	}

	if (NearestTarget)
	{
		AIController->SetNewTarget(NearestTarget);
	}
}
