#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Player/Guardian/GS_Drakhar.h"

UGS_DrakharAnimInstance::UGS_DrakharAnimInstance()
{
	
}

void UGS_DrakharAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		Drakhar = Cast<AGS_Drakhar>(Owner);
	}
}

void UGS_DrakharAnimInstance::AnimNotify_ComboAttack()
{
	Drakhar->MeleeAttackCheck();
}

void UGS_DrakharAnimInstance::AnimNotify_Reset()
{
	Drakhar->ServerRPCResetValue();
}

void UGS_DrakharAnimInstance::AnimNotify_ShootEnergy()
{
	Drakhar->ComboLastAttack();
}

void UGS_DrakharAnimInstance::AnimNotify_EarthquakeCheck()
{	
	Drakhar->ServerRPCEarthquakeAttackCheck();
}

void UGS_DrakharAnimInstance::AnimNotify_DraconicFury()
{
	Drakhar->ServerRPCSpawnDraconicFury();
}

void UGS_DrakharAnimInstance::AnimNotify_CtrlSkillEnd()
{
	Drakhar->ServerRPCStopCtrl();
}

void UGS_DrakharAnimInstance::AnimNotify_FinishCtrlSkill()
{
	Drakhar->FinishCtrlSkill();
}
