#include "Character/Skill/Guardian/Drakhar/GS_DrakharFly.h"

#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillComp.h"

UGS_DrakharFly::UGS_DrakharFly()
{
	bIsFlying = false;
	// Cooltime = 5.f;
	// bIsFlying = false;
	//
	// static ConstructorHelpers::FObjectFinder<UAnimMontage> FlyMontage(TEXT("/Game/Player/Guardian/Drakhar/Animations/Blueprint/AM_DraconicFury1.AM_DraconicFury1"));
	// if (FlyMontage.Succeeded())
	// {
	// 	SkillAnimMontages.Add(FlyMontage.Object);
	// }
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
	
	UE_LOG(LogTemp,Warning,TEXT("#########################ACTIVE"));
	bIsFlying = true;
	
	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, true);
	}
	
	ExecuteSkillEffect();
}

void UGS_DrakharFly::DeactiveSkill()
{
	UE_LOG(LogTemp,Warning,TEXT("#########################DEACTIVE"));

	bIsFlying = false;

	if (OwnerCharacter && OwnerCharacter->GetSkillComp())
	{
		OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ready, false);
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
