// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GS_Character.h"
#include "BehaviorTree/BehaviorTree.h"       
#include "BehaviorTree/BlackboardData.h"
#include "AkGameplayStatics.h"
#include "GS_Monster.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_Monster : public AGS_Character
{
	GENERATED_BODY()

public:
	AGS_Monster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AvoidanceRadius = 200.0f;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BTAsset;
	
	UPROPERTY(EditAnywhere, Category = "AI")
	UBlackboardData* BBAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	UAkAudioEvent* ClickSoundEvent;

	void SetSelected(bool bIsSelected);

protected:
	UPROPERTY(VisibleAnywhere)
	UDecalComponent* SelectionDecal;

	UPROPERTY(VisibleAnywhere)
	UAkComponent* AkComponent;

};

