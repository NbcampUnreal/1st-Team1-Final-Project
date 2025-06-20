#include "Character/Skill/Guardian/Drakhar/GS_DrakharWingRush.h"
#include "Character/Player/GS_Player.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Templates/SharedPointer.h"

UGS_DrakharWingRush::UGS_DrakharWingRush()
{
	CurrentSkillType = ESkillSlot::Moving;
}

void UGS_DrakharWingRush::ActiveSkill()
{
	Super::ActiveSkill();
	
	ExecuteSkillEffect();
}

void UGS_DrakharWingRush::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();

	OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
}

