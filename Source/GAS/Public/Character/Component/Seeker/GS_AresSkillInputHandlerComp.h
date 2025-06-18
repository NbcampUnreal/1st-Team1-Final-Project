// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "GS_AresSkillInputHandlerComp.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGS_AresSkillInputHandlerComp : public UGS_SkillInputHandlerComp
{
	GENERATED_BODY()
	
protected:
	virtual void OnRightClick(const struct FInputActionInstance& Instance) override;
	virtual void OnLeftClick(const struct FInputActionInstance& Instance) override;
	virtual void OnLeftClickRelease(const struct FInputActionInstance& Instance) override;
};
