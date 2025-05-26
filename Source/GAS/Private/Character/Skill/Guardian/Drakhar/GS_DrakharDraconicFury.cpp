#include "Character/Skill/Guardian/Drakhar/GS_DrakharDraconicFury.h"

#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
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
	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter);
	if (IsValid(Drakhar))
	{
		Drakhar->ResetComboAttackVariables();
	}
	ExecuteSkillEffect();
}

void UGS_DrakharDraconicFury::ExecuteSkillEffect()
{
	StartCoolDown();
	//OwnerCharacter->GetSkillComp()->StartTimer(ESkillSlot::Ultimate);
	OwnerCharacter->MulticastRPCPlaySkillMontage((SkillAnimMontages[0]));
}
