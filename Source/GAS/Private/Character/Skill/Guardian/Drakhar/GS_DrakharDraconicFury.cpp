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
	Super::ActiveSkill();
	
	if (!CanActive())
	{
		return;
	}
	ExecuteSkillEffect();
}

void UGS_DrakharDraconicFury::ExecuteSkillEffect()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	//server logic
	AGS_Guardian* Guardian = Cast<AGS_Guardian>(OwnerCharacter);
	if (Guardian)
	{
		Guardian->GuardianDoSkillState = EGuardianDoSkill::Ultimate;	
	}
	
	StartCoolDown();

	if (OwnerCharacter)
	{
		//play montage, except server
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
	
}
