// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "GS_NeedleFang.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_NeedleFang : public AGS_Monster
{
	GENERATED_BODY()

public:
	AGS_NeedleFang();

protected:
	virtual void BeginPlay() override;
};
