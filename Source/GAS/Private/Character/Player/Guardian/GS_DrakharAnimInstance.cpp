// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Player/Guardian/GS_Guardian.h"

UGS_DrakharAnimInstance::UGS_DrakharAnimInstance()
{
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
	Montage_Stop(0.2f, ComboAttackMontages[InCurrentComboIndex]);
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

void UGS_DrakharAnimInstance::AnimNotify_ComboAttackCheckStart()
{
	AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner);
		if (IsValid(Drakhar))
		{
			Drakhar->ServerRPCComboAttackUpdate();
		}
	}
}

void UGS_DrakharAnimInstance::AnimNotify_ComboAttackCheckEnd()
{
	AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner);
		if (IsValid(Drakhar))
		{
			Drakhar->ServerRPCComboAttackEnd();
		}
	}
}

void UGS_DrakharAnimInstance::AnimNotify_DashHitCheck()
{
	
}

void UGS_DrakharAnimInstance::AnimNotify_EarthquakeCheck()
{
	AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner);
		if (IsValid(Drakhar))
		{
			Drakhar->ServerRPCEarthquakeAttackCheck();
		}
	}
}

void UGS_DrakharAnimInstance::AnimNotify_EarthquakeCheckEnd()
{
	AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner);
		if (IsValid(Drakhar))
		{
			Drakhar->ServerRPCEarthquakeEnd();
		}
	}
}

