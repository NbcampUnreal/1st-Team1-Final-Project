// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Character/Component/GS_FootManagerComponent.h"
#include "GS_AnimNotifyFoleyEvent.generated.h"

/**
 * 기존 FoleyEvent를 확장한 발걸음 애니메이션 노티파이
 * 애니메이션 상태와 발의 위치를 기반으로 자동으로 왼발/오른발 감지
 */
UCLASS(const, hidecategories=Object, collapsecategories, meta=(DisplayName="Foley Event (Footstep)"))
class GAS_API UGS_AnimNotifyFoleyEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UGS_AnimNotifyFoleyEvent();

	/** 애니메이션 노티파이 실행 */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 에디터에서 노티파이 이름 표시 */
	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** 발걸음 감지 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foley", meta = (DisplayName = "Enable Footstep"))
	bool bEnableFootstep = true;

	/** 발 감지 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foley", meta = (DisplayName = "Detection Method"))
	EFootDetectionMethod DetectionMethod = EFootDetectionMethod::VelocityBased;

	/** 발 속도 임계값 (Velocity 기반 감지용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foley", meta = (DisplayName = "Velocity Threshold", ClampMin = "0.1", ClampMax = "10.0"))
	float VelocityThreshold = 2.0f;

private:
	/** 발걸음을 자동 감지하여 처리 */
	void TriggerAutoFootstep(USkeletalMeshComponent* MeshComp);
	
	/** 발의 속도를 기반으로 어느 발이 땅에 닿았는지 감지 */
	EFootStep DetectFootByVelocity(USkeletalMeshComponent* MeshComp);
	
	/** 발의 높이를 기반으로 어느 발이 땅에 닿았는지 감지 */
	EFootStep DetectFootByHeight(USkeletalMeshComponent* MeshComp);
}; 