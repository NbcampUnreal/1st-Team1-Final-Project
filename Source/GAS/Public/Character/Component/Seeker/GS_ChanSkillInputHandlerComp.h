// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "GS_ChanSkillInputHandlerComp.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_ChanSkillInputHandlerComp : public UGS_SkillInputHandlerComp
{
	GENERATED_BODY()
	
protected:
	virtual void OnRightClick(const struct FInputActionInstance& Instance) override;
	virtual void OnLeftClick(const struct FInputActionInstance& Instance) override;
	virtual void OnRoll(const struct FInputActionInstance& Instance) override;
	virtual void OnKeyReset(const struct FInputActionInstance& Instance) override;
};
