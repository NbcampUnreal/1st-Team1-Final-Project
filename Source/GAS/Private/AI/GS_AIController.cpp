// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/GS_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName AGS_AIController::HomePosKey(TEXT("HomePos"));
const FName AGS_AIController::TargetKey(TEXT("Target"));

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
	
	UBlackboardComponent* blackboardComponent = Blackboard;
    
	if (UseBlackboard(BBAsset, blackboardComponent))
    {
    	blackboardComponent->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());
    	RunBehaviorTree(BTAsset);
    }
}

void AGS_AIController::TargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}
