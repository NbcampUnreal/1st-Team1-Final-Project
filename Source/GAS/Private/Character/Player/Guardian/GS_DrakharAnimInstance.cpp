// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Character/Player/Guardian/GS_Guardian.h"

UGS_DrakharAnimInstance::UGS_DrakharAnimInstance()
{
}

void UGS_DrakharAnimInstance::PlayAttackMontage()
{
	Montage_Play(AttackMontage, 1.f);
}

void UGS_DrakharAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
	Montage_JumpToSection(GetAttackMontageSectionName(NewSection), AttackMontage);
}

void UGS_DrakharAnimInstance::AnimNotify_AttackHitCheck()
{
	OnAttackHitCheck.Broadcast();

	AActor* Owner = GetOwningActor();
	if (IsValid(Owner))
	{
		AGS_Guardian* Guardian = Cast<AGS_Guardian>(Owner);
		if (IsValid(Guardian))
		{
			Guardian->MeleeAttackCheck();
		}
	}
}

void UGS_DrakharAnimInstance::AnimNotify_NextAttackCheck()
{
	OnNextAttackCheck.Broadcast();
}


FName UGS_DrakharAnimInstance::GetAttackMontageSectionName(int32 Section)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString::Printf(TEXT("Attack%d"), Section));

	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}
