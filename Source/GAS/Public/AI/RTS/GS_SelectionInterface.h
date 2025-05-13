// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GS_SelectionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGS_SelectionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_API IGS_SelectionInterface
{
	GENERATED_BODY()

public:
	// 드래그 시작
	virtual void StartSelection() = 0;
	// 드래그 종료
	virtual void StopSelection() = 0;
	
};
