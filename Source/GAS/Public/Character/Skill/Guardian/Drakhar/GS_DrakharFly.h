#pragma once

#include "CoreMinimal.h"
#include "Character/Skill/GS_SkillBase.h"
#include "GS_DrakharFly.generated.h"


UCLASS()
class GAS_API UGS_DrakharFly : public UGS_SkillBase
{
	GENERATED_BODY()

public:
	UGS_DrakharFly();
	
	virtual void ActiveSkill() override;
	virtual void DeactiveSkill() override;
	virtual void ExecuteSkillEffect() override;

private:
	bool bIsFlying;
};
