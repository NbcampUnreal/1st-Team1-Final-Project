// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_DrakharDraconicFury.generated.h"

UCLASS()
class GAS_API UGS_DrakharDraconicFury : public UGS_SkillBase
{
	GENERATED_BODY()
	
public:
	UGS_DrakharDraconicFury();

	virtual void ActiveSkill() override;
	virtual void ExecuteSkillEffect() override;
	virtual void OnSkillAnimationEnd() override;

private:
	// === 인디케이터 관리 ===
	FTimerHandle IndicatorTimerHandle; // 인디케이터 타이머 핸들

	int32 CurrentIndicatorIndex; // 현재 표시 중인 인디케이터 인덱스 (일반 모드용)

	// === 스킬 설정값 ===
	// 인디케이터 표시 간격 (초) - 순차적으로 표시할 때 사용
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Indicator", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float IndicatorDisplayInterval = 0.15f;

	// 인디케이터 기본 반경 (단위: cm)
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Indicator", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float IndicatorRadius = 250.0f;

	// 피버 모드 인디케이터 반경 배율
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Indicator|Fever", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float FeverIndicatorRadiusMultiplier = 1.5f;
};
