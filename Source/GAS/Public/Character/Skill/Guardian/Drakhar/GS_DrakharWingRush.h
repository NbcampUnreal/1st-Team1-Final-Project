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
};
