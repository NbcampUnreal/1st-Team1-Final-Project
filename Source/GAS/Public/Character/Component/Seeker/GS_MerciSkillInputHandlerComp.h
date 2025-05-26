// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "GS_MerciSkillInputHandlerComp.generated.h"

class AGS_SeekerMerciArrow;
/**
 * 
 */
UCLASS()
class GAS_API UGS_MerciSkillInputHandlerComp : public UGS_SkillInputHandlerComp
{
	GENERATED_BODY()

protected:
	UGS_MerciSkillInputHandlerComp();

	TSubclassOf<AGS_SeekerMerciArrow> ArrowClass;
	UAnimMontage* SkillAnimMontages;

	virtual void OnRightClick(const struct FInputActionInstance& Instance) override;
	virtual void OnLeftClick(const struct FInputActionInstance& Instance) override;
	virtual void OnRightClickRelease(const struct FInputActionInstance& Instance) override;
	virtual void OnLeftClickRelease(const struct FInputActionInstance& Instance) override;
};
