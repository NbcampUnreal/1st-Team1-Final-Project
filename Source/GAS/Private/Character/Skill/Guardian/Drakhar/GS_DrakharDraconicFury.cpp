#include "Character/Skill/Guardian/Drakhar/GS_DrakharDraconicFury.h"
#include "Character/Player/GS_Player.h"
#include "Character/Skill/GS_SkillComp.h"

UGS_DrakharDraconicFury::UGS_DrakharDraconicFury()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}

void UGS_DrakharDraconicFury::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}
	ExecuteSkillEffect();
}

void UGS_DrakharDraconicFury::InterruptSkill()
{
	Super::InterruptSkill();
}

void UGS_DrakharDraconicFury::ExecuteSkillEffect()
{
	StartCoolDown();
	OwnerCharacter->MulticastRPCPlaySkillMontage((SkillAnimMontages[0]));
}
