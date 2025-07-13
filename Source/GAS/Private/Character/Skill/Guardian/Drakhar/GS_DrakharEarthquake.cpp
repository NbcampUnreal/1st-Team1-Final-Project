#include "Character/Skill/Guardian/Drakhar/GS_DrakharEarthquake.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"

UGS_DrakharEarthquake::UGS_DrakharEarthquake()
{
	CurrentSkillType = ESkillSlot::Aiming;
}

void UGS_DrakharEarthquake::ActiveSkill()
{
	if (!CanActive())
	{
		return;
	}
	
	ExecuteSkillEffect();
}

void UGS_DrakharEarthquake::ExecuteSkillEffect()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	//server logic
	bIsEarthquaking = true;
	
	StartCoolDown();
	
	if (OwnerCharacter)
	{
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
	
	AGS_Guardian* Guardian =Cast<AGS_Guardian>(OwnerCharacter);
	if (Guardian)
	{
		Guardian->GuardianDoSkillState = EGuardianDoSkill::Aiming;
	}
}
