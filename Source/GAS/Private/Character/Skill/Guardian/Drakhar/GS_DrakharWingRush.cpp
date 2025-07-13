#include "Character/Skill/Guardian/Drakhar/GS_DrakharWingRush.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"
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

	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	//server logic
	if (OwnerCharacter)
	{
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
	
	AGS_Guardian* Guardian = Cast<AGS_Guardian>(OwnerCharacter);
	if (Guardian)
	{
		Guardian->GuardianDoSkillState = EGuardianDoSkill::Moving;	
	}
}

