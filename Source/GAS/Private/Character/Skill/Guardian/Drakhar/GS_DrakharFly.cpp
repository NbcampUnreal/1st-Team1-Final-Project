#include "Character/Skill/Guardian/Drakhar/GS_DrakharFly.h"

#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
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
	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter);
	if (IsValid(Drakhar))
	{
		Drakhar->ResetComboAttackVariables();
	}
	//UE_LOG(LogTemp,Warning,TEXT("#########################ACTIVE"));
	bIsFlying = true;
	
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, true);
	}
	ExecuteSkillEffect();
}

void UGS_DrakharFly::DeactiveSkill()
{
	//UE_LOG(LogTemp,Warning,TEXT("#########################DEACTIVE"));

	bIsFlying = false;

	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, false);
	}
	
	ExecuteSkillEffect();
	//StartCoolDown();
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
