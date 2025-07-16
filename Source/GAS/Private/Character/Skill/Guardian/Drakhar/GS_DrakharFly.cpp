#include "Character/Skill/Guardian/Drakhar/GS_DrakharFly.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"

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
	
	if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter))
	{
		Drakhar->MulticastRPC_OnFlyStart();
	}
	
	ExecuteSkillEffect();
}

void UGS_DrakharFly::OnSkillCanceledByDebuff()
{
	bIsFlying = false;
	
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, false);
	}
	
	if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter))
	{
		Drakhar->MulticastRPC_OnFlyEnd();
		Drakhar->ClientGuardianDoSkillState = EGuardianDoSkill::None;
		Drakhar->GuardianDoSkillState = EGuardianDoSkill::None;
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
