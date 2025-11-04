// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Chan/GS_AN_ChanSwitchingAxeSlot.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"

void UGS_AN_ChanSwitchingAxeSlot::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AGS_Chan* Chan = Cast<AGS_Chan>(MeshComp->GetOwner());

	if (!Chan)
	{
		return;
	}
	
	AGS_WeaponAxe* Axe = Cast<AGS_WeaponAxe>(Chan->GetWeaponByIndex(0)); // hard coding

	if (!Axe)
	{
		return;
	}

	if (TargetAxesocket == ESwitchingAxeSocket::Sheathing)
	{
		Axe->AttachToComponent(
		MeshComp,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("AxeSheath"));

		Chan->SetWeaponHandlingState(EWeaponHandlingState::Sheathing);
	}
	else if (TargetAxesocket == ESwitchingAxeSocket::Wielding)
	{
		Axe->AttachToComponent(
		MeshComp,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("Axe"));

		Chan->SetWeaponHandlingState(EWeaponHandlingState::Wielding);
	}
}
