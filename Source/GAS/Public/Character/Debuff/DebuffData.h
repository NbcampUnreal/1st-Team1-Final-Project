// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDebuffType.h"
#include "DebuffData.generated.h"

class UGS_DebuffBase;

USTRUCT(BlueprintType)
struct GAS_API FDebuffData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDebuffType DebuffType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGS_DebuffBase> DebuffClass;
};
