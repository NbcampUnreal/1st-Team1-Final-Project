// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName AGS_AIController::HomePosKey(TEXT("HomePos"));
const FName AGS_AIController::TargetActorKey(TEXT("Target"));
const FName AGS_AIController::RTSTargetKey(TEXT("RTSTarget"));
const FName AGS_AIController::bUseRTSKey(TEXT("bUseRTS"));
const FName AGS_AIController::TargetLockedKey(TEXT("bTargetLocked"));

AGS_AIController::AGS_AIController()
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
	if (Blackboard->GetValueAsBool(TargetLockedKey))
	{
		return;
	}

	// 현재위치 가장 가까운 애한테 타겟을 걸도록 ! 
	if (Stimulus.WasSuccessfullySensed())
	{
		Blackboard->SetValueAsObject(TargetActorKey, Actor);
	}
	else
	{
		Blackboard->ClearValue(TargetActorKey);
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
	PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
}

void AGS_AIController::ExitConfuseState()
{
	PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), true);
	
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
