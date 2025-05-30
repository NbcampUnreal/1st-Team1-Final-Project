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

void UGS_DrakharAnimInstance::PlayComboAttackMontage(int32 InCurrentComboIndex)
{
	if (InCurrentComboIndex >= ComboAttackMontages.Num())
	{
		return;
	}
	Montage_Play(ComboAttackMontages[InCurrentComboIndex], 1.f);
}

void UGS_DrakharAnimInstance::StopComboAttackMontage(int32 InCurrentComboIndex)
{
	Montage_Stop(0.f, ComboAttackMontages[InCurrentComboIndex]);
}

void UGS_DrakharAnimInstance::AnimNotify_ComboAttackCheck()
{	
	Drakhar->ServerRPCComboAttackCheck();	
}

void UGS_DrakharAnimInstance::AnimNotify_ComboAttackEnd()
{
	Drakhar->ServerRPCComboAttackEnd();
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
