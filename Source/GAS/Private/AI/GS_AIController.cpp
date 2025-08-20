// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GS_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName AGS_AIController::HomePosKey(TEXT("HomePosition"));
const FName AGS_AIController::MoveLocationKey(TEXT("MoveLocation"));
const FName AGS_AIController::TargetActorKey(TEXT("TargetActor"));
const FName AGS_AIController::CommandKey(TEXT("RTSCommand"));
const FName AGS_AIController::TargetLockedKey(TEXT("bTargetLocked"));
const FName AGS_AIController::DebuffLockedKey(TEXT("bDebuffLocked"));
const FName AGS_AIController::CanAttackKey(TEXT("bCanAttack"));
const FName AGS_AIController::LastAttackTimeKey(TEXT("LastAttackTime"));

AGS_AIController::AGS_AIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>("PathFollowingComponent"))
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	PerceptionComponent->ConfigureSense(*SightConfig);
	
    SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals  = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies= false;
}

void AGS_AIController::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AGS_AIController::TargetPerceptionUpdated);
	
	if (UCrowdFollowingComponent* CrowdComp = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
	{
		CrowdComp->SetCrowdSimulationState(ECrowdSimulationState::Enabled);
		CrowdComp->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Low);
		CrowdComp->SetAvoidanceGroup(1);
		CrowdComp->SetGroupsToAvoid(1);
		CrowdComp->SetCrowdCollisionQueryRange(200.0f);
		CrowdComp->SetCrowdPathOptimizationRange(100.0f);
		CrowdComp->SetCrowdSeparation(true);
		CrowdComp->SetCrowdSeparationWeight(0.2f);
		
		CrowdComp->SetCrowdOptimizeVisibility(false);
		CrowdComp->SetCrowdOptimizeTopology(false);
		CrowdComp->SetCrowdRotateToVelocity(false);
	}
}

void AGS_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (AGS_Monster* Monster = Cast<AGS_Monster>(InPawn))
	{
		BTAsset = Monster->BTAsset;
		BBAsset = Monster->BBAsset;
	}

	UBlackboardComponent* BlackboardComponent = Blackboard;
	if (BBAsset && UseBlackboard(BBAsset, BlackboardComponent))
	{
		BlackboardComponent->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());

		if (BTAsset)
		{
			RunBehaviorTree(BTAsset);
		}
	}

	SetGenericTeamId(GetGenericTeamId());
}

FGenericTeamId AGS_AIController::GetGenericTeamId() const
{
	if (AGS_Character* Char = Cast<AGS_Character>(GetPawn()))
	{
		return Char->TeamId;
	}
	return FGenericTeamId::NoTeam;
}

void AGS_AIController::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Blackboard->GetValueAsBool(DebuffLockedKey))
	{
		return;
	}
	
	if (Blackboard->GetValueAsBool(TargetLockedKey))
	{
		return;
	}

	// 지금 시야 감지 범위 안에 있는 타겟들
	TArray<AActor*> Targets;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Targets);

	if (Targets.IsEmpty()) 
	{
		if (Blackboard->GetValueAsObject(TargetActorKey) != nullptr)
		{
			ClearCurrentTarget();
		}
		
		return;
	}
	
	// 가장 가까운 타겟 찾기
	APawn* ControlledPawn = GetPawn();
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
		SetNewTarget(NearestTarget);
	}
}

void AGS_AIController::SetNewTarget(AActor* NewTarget)
{
	if (NewTarget)
	{
		Blackboard->SetValueAsObject(TargetActorKey, NewTarget);
        
		// 새 타겟인 경우 델리게이트 연결
		AGS_Character* NewTargetCharacter = Cast<AGS_Character>(NewTarget);
		if (NewTargetCharacter && TargetCharacter.Get() != NewTargetCharacter)
		{
			if (TargetCharacter.IsValid())
			{
				TargetCharacter->OnDeathDelegate.RemoveDynamic(this, &AGS_AIController::OnTargetDied);
			}
			
			TargetCharacter = NewTargetCharacter;
			if (!NewTargetCharacter->IsDead())
			{
				TargetCharacter->OnDeathDelegate.AddDynamic(this, &AGS_AIController::OnTargetDied);
			}
		}
	}
}

void AGS_AIController::OnTargetDied()
{
	if (Blackboard)
	{
		Blackboard->ClearValue(TargetActorKey);
		Blackboard->SetValueAsEnum(CommandKey, 0);
	}
	
	TargetCharacter = nullptr;
}

void AGS_AIController::ClearCurrentTarget()
{
	Blackboard->ClearValue(TargetActorKey);
	
	if (TargetCharacter.IsValid())
	{
		TargetCharacter->OnDeathDelegate.RemoveDynamic(this, &AGS_AIController::OnTargetDied);
		TargetCharacter = nullptr;
	}
}


void AGS_AIController::LockTarget(AGS_Character* Target)
{
	Blackboard->SetValueAsObject(TargetActorKey, Target);
	Blackboard->SetValueAsBool(TargetLockedKey, true);
}

void AGS_AIController::UnlockTarget()
{
	Blackboard->SetValueAsBool(TargetLockedKey, false);
}

void AGS_AIController::EnterConfuseState()
{
	PrevTargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey));
	Blackboard->ClearValue(TargetActorKey);
	Blackboard->SetValueAsBool(DebuffLockedKey, true);
	PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
}

void AGS_AIController::ExitConfuseState()
{
	PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	Blackboard->SetValueAsBool(DebuffLockedKey, false);

	if (PrevTargetActor.IsValid())
	{
		Blackboard->SetValueAsObject(TargetActorKey, PrevTargetActor.Get());
	}
	else
	{
		PerceptionComponent->RequestStimuliListenerUpdate();
	}
	PrevTargetActor.Reset();
}
