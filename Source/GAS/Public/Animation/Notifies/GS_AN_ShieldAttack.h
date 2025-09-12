// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GS_AN_ShieldAttack.generated.h"

/**
 * 방패 공격 콜리전을 활성화하는 애니메이션 노티파이
 */
UCLASS()
class GAS_API UGS_AN_ShieldAttack : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UGS_AN_ShieldAttack();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// 방패 공격 콜리전 활성화 지속 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield Attack", meta = (AllowPrivateAccess = "true"))
	float AttackDuration = 0.5f;
};
