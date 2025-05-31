// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BT/GS_BTS_Cooldown.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"

UGS_BTS_Cooldown::UGS_BTS_Cooldown()
{
	NodeName = TEXT("Cooldown");
	bNotifyTick = true;
}

void UGS_BTS_Cooldown::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}
	
	AGS_Character* Character = Cast<AGS_Character>(AIController->GetPawn());
	if (!Character)
	{
		return;
	}
	
	const float NowTime = OwnerComp.GetWorld()->GetTimeSeconds();
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const float LastTime = Blackboard->GetValueAsFloat(TEXT("LastAttackTime"));
	const float AttackSpeed = Character->GetStatComp()->GetAttackSpeed();
	const bool bCanAttack = (NowTime - LastTime) >= AttackSpeed;
	
	Blackboard->SetValueAsBool(TEXT("bCanAttack"), bCanAttack);
	
}
