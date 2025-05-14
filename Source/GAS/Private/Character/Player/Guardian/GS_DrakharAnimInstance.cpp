// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Player/Guardian/GS_Guardian.h"

UGS_DrakharAnimInstance::UGS_DrakharAnimInstance()
{
}

void UGS_DrakharAnimInstance::PlayAttackMontage()
{
	if (AttackMontage)
	{
		Montage_Play(AttackMontage, 1.f);
	}
}

void UGS_DrakharAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
	Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

void UGS_DrakharAnimInstance::PlayDashMontage()
{
	if (DashMontage)
	{
		float MontageLength = DashMontage->GetPlayLength();
		float PlayRate = MontageLength > 0 ? MontageLength / 0.6f : 1.f;
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


void UGS_DrakharAnimInstance::AnimNotify_AttackHitCheck()
{
	OnAttackHitCheck.Broadcast();

	/*AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(Owner);
		if (IsValid(Guardian))
		{
			Guardian->MeleeAttackCheck();
		}
	}*/
}

void UGS_DrakharAnimInstance::AnimNotify_NextAttackCheck()
{
	OnNextAttackCheck.Broadcast();
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


FName UGS_DrakharAnimInstance::GetAttackMontageSectionName(int32 Section)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString::Printf(TEXT("Attack%d"), Section));

	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}
