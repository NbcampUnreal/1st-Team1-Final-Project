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
	Super::ActiveSkill();
	
	if (!CanActive())
	{
		return;
	}
	if (bIsFlying)
	{
		return;
	}

	bIsFlying = true;
	
	// 멀티플레이어 환경에서 안전성 체크 추가
	if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter))
	{
		if (IsValid(Drakhar))
		{
			Drakhar->MulticastRPC_OnFlyStart();
		}
	}
	
	ExecuteSkillEffect();
}

void UGS_DrakharFly::OnSkillCanceledByDebuff()
{
	bIsFlying = false;
	
	if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter))
	{
		if (IsValid(Drakhar))
		{
			Drakhar->MulticastRPC_OnFlyEnd();
			Drakhar->GuardianDoSkillState = EGuardianDoSkill::None;
			Drakhar->GuardianState = EGuardianCtrlState::CtrlEnd;
		}
	}
	ExecuteSkillEffect();
}


void UGS_DrakharFly::ExecuteSkillEffect()
{
	// 멀티플레이어 환경에서 안전성 체크 추가
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	// 애니메이션 몽타주 유효성 체크
	if (SkillAnimMontages.Num() == 0 || !SkillAnimMontages[0])
	{
		return;
	}

	if (bIsFlying)
	{
		OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
	}
	else
	{
		OwnerCharacter->MulicastRPCStopCurrentSkillMontage(SkillAnimMontages[0]);
	}
}
