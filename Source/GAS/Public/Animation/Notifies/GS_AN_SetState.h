// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/GS_TpsController.h"
#include "Animation/Character/E_SeekerAnim.h"
#include "GS_AN_SetState.generated.h"

UCLASS()
class GAS_API UGS_AN_SetState : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESeekerMontageSlot SeekerSlot = ESeekerMontageSlot::End;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChangeSeekerGait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanAcceptComboInput;

	UPROPERTY(EditAnywhere)
	bool bUseControlValue = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition="bUseControlValue"))
	FControlValue ControlValue;

	UPROPERTY(EditAnywhere)
	bool bChangeSeekerGait = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition="bChangeSeekerGait"))
	EGait Gait = EGait::Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanResetAllowedSkills = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition="bCanResetAllowedSkills"))
	bool bResetAllowedSkills;
};

