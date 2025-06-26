#include "Character/Skill/Guardian/Drakhar/GS_DrakharDraconicFury.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"

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

void UGS_DrakharDraconicFury::ExecuteSkillEffect()
{
	StartCoolDown();

	if (OwnerCharacter)
	{
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
	
	AGS_Guardian* Guardian =Cast<AGS_Guardian>(OwnerCharacter);
	if (Guardian)
	{
		Guardian->ClientGuardianDoSkillState = EGuardianDoSkill::Ultimate;
		Guardian->GuardianDoSkillState = EGuardianDoSkill::Ultimate;	
	}
}
