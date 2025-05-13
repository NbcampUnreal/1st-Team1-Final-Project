// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName AGS_AIController::HomePosKey(TEXT("HomePos"));
const FName AGS_AIController::TargetActorKey(TEXT("Target"));
const FName AGS_AIController::RTSTargetKey(TEXT("RTSTarget"));
const FName AGS_AIController::bUseRTSKey(TEXT("bUseRTS"));

AGS_AIController::AGS_AIController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	PerceptionComponent->ConfigureSense(*SightConfig);
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
}

void AGS_AIController::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		Blackboard->SetValueAsObject(TargetActorKey, Actor);
	}
	else
	{
		Blackboard->ClearValue(TargetActorKey);
	}
}
