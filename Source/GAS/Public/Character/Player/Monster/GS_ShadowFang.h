// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "GS_ShadowFang.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_ShadowFang : public AGS_Monster
{
	GENERATED_BODY()

public:
	AGS_ShadowFang();

protected:
	virtual void BeginPlay() override;

};
