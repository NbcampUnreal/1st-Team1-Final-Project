#include "Character/Skill/Guardian/Drakhar/GS_DrakharDraconicFury.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"

UGS_DrakharDraconicFury::UGS_DrakharDraconicFury()
{
	Cooltime = 45.f;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DraconicFuryMontage(TEXT("/Game/Player/Guardian/Drakhar/Animations/Blueprint/AM_DraconicFury2.AM_DraconicFury2"));
	if (DraconicFuryMontage.Succeeded())
	{
		SkillAnimMontages.Add(DraconicFuryMontage.Object);
	}
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
	OwnerCharacter->MulticastRPCPlaySkillMontage((SkillAnimMontages[0]));
}
