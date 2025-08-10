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
	Super::ActiveSkill();

	//cool time check
	if (!CanActive())
	{
		return;
	}
	
	ExecuteSkillEffect();
}

void UGS_DrakharEarthquake::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();
	
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	//server logic
	AGS_Guardian* Guardian =Cast<AGS_Guardian>(OwnerCharacter);
	if (Guardian)
	{
		Guardian->GuardianDoSkillState = EGuardianDoSkill::Aiming;
	}
	
	StartCoolDown();
	
	if (OwnerCharacter)
	{
		//play montage, except server
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
}
