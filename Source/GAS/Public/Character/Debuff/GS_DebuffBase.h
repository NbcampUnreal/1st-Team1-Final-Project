// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Character/Debuff/EDebuffType.h"
#include "GS_DebuffBase.generated.h"

class AGS_Character;
/**
 * 
 */
UCLASS()
class GAS_API UGS_DebuffBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(AGS_Character* InTarget, float InDuration, int32 InPriority, EDebuffType InDebuffType);
	virtual void OnApply();
	virtual void OnExpire();
	
	int32 GetPriority() const { return Priority; }
	float GetDuration() const { return Duration; }
	EDebuffType GetDebuffType() const { return DebuffType; }
	float GetRemainingTime(float CurrentTime) const;

	UPROPERTY()
	float StartTime;
protected:
	UPROPERTY()
	AGS_Character* TargetCharacter;

	UPROPERTY()
	EDebuffType DebuffType;

	float Duration;
	int32 Priority;

};
