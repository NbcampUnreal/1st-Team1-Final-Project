// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Player/Guardian/GS_Guardian.h"

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

void UGS_DrakharAnimInstance::PlayDashMontage()
{
	if (DashMontage)
	{
		Montage_Play(DashMontage, 1.f);
	}
}

void UGS_DrakharAnimInstance::StopDashMontage()
{
	if (DashMontage)
	{
		Montage_Stop(0.2f, DashMontage);
	}
}

void UGS_DrakharAnimInstance::PlayEarthquakeMontage()
{
	if (EarthquakeMontage)
	{
		Montage_Play(EarthquakeMontage, 1.f);
	}
}

void UGS_DrakharAnimInstance::StopEarthquakeMontage()
{
	if (EarthquakeMontage)
	{
		Montage_Stop(0.2f, EarthquakeMontage);
	}
}

void UGS_DrakharAnimInstance::PlayDraconicFuryMontage(int32 InPlayIndex)
{
	if (InPlayIndex >= DraconicFuryMontages.Num())
	{
		return;
	}
	Montage_Play(DraconicFuryMontages[InPlayIndex], 1.f);
}

void UGS_DrakharAnimInstance::StopDraconicFuryMontage(int32 InPlayIndex)
{
	Montage_Stop(0.1f, DraconicFuryMontages[InPlayIndex]);
}

void UGS_DrakharAnimInstance::AnimNotify_ComboAttackCheck()
{	
	Drakhar->ServerRPCComboAttackCheck();	
}

void UGS_DrakharAnimInstance::AnimNotify_ComboAttackEnd()
{
	Drakhar->ServerRPCComboAttackEnd();
}

void UGS_DrakharAnimInstance::AnimNotify_DashHitCheck()
{
	//Drakhar->ServerRPCComboAttackEnd();
}

void UGS_DrakharAnimInstance::AnimNotify_EarthquakeCheck()
{	
	Drakhar->ServerRPCEarthquakeAttackCheck();
}

void UGS_DrakharAnimInstance::AnimNotify_EarthquakeCheckEnd()
{
	Drakhar->ServerRPCEarthquakeEnd();	
}

