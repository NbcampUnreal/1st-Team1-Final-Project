// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GS_AIController.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_AIController : public AAIController
{
	GENERATED_BODY()

public:
	AGS_AIController();

	static const FName TargetKey;
};
