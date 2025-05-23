// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/DataAsset.h"
#include "MonsterDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAS_API UMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	UTexture2D* Portrait;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	FText MonsterName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	FText TypeName;
};
