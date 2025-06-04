#include "Character/Skill/Guardian/Drakhar/GS_DrakharFly.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"

UGS_DrakharFly::UGS_DrakharFly()
{
	bIsFlying = false;

	CurrentSkillType = ESkillSlot::Ready;
}

void UGS_DrakharFly::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}
	if (bIsFlying)
	{
		return;
	}
	bIsFlying = true;
	
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, true);
	}
	ExecuteSkillEffect();
}

void UGS_DrakharFly::DeactiveSkill()
{
	bIsFlying = false;

	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, false);
	}
	
	ExecuteSkillEffect();
}

void UGS_DrakharFly::ExecuteSkillEffect()
{
	if (bIsFlying)
	{
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
	else
	{
		OwnerCharacter->MulicastRPCStopCurrentSkillMontage(SkillAnimMontages[0]);
	}
}
