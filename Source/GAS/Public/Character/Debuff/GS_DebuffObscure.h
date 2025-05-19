// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Debuff/GS_DebuffBase.h"
#include "GS_DebuffObscure.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_DebuffObscure : public UGS_DebuffBase
{
	GENERATED_BODY()
	
public:
	virtual void OnApply() override;
	virtual void OnExpire() override;
};
