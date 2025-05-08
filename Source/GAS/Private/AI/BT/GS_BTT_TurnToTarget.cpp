// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTT_TurnToTarget.h"
#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GS_Character.h"

UGS_BTT_TurnToTarget::UGS_BTT_TurnToTarget()
{
	NodeName = TEXT("TurnToTarget");
	bNotifyTick = true;
	
	RotationSpeed = 360.f;
	AcceptanceAngle = 5.f;
}

EBTNodeResult::Type UGS_BTT_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AGS_AIController* AIController = Cast<AGS_AIController>(OwnerComp.GetAIOwner());
	if(!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AGS_Character* TargetActor = Cast<AGS_Character>(Blackboard->GetValueAsObject(AGS_AIController::TargetKey));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress;
}

void UGS_BTT_TurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AGS_AIController* AIController = Cast<AGS_AIController>(OwnerComp.GetAIOwner());
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AGS_Character* TargetActor = Cast<AGS_Character>(Blackboard->GetValueAsObject(AGS_AIController::TargetKey));
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	FVector Dir = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();
	Dir.Z = 0.f;

	// 목표 회전, 보간 
	const FRotator DesiredRot = Dir.Rotation();
	const FRotator CurrentRot = ControlledPawn->GetActorRotation();
	const FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, DesiredRot, DeltaSeconds, RotationSpeed);
	ControlledPawn->SetActorRotation(NewRot);
	
	const float DeltaYaw = FMath::Abs(FRotator::NormalizeAxis(DesiredRot.Yaw - NewRot.Yaw));
	if (DeltaYaw <= AcceptanceAngle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
