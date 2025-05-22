#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_DrakharWingRush.generated.h"

UCLASS()
class GAS_API UGS_DrakharWingRush : public UGS_SkillBase
{
	GENERATED_BODY()
public:
	UGS_DrakharWingRush();

	virtual void ActiveSkill() override;
	virtual void ExecuteSkillEffect() override;

	// UFUNCTION()
	// void DashTimerEnd();
	// int32 DashMoveSetting();
	// void DoDash();

	
protected:
	FTimerHandle DashTimer;
	FVector DashStartLocation;
	FVector DashEndLocation;
	
	float DashRemainTime;
	float DashPower;

	//float DashInterpAlpha;
	//int32 Activeid;

	// UFUNCTION(BlueprintImplementableEvent)
	// void OnPreciseMoveStarted(float Duration);
	//
};
