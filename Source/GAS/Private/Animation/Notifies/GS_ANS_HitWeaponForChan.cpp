// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_HitWeaponForChan.h"
#include "Character/GS_Character.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"

void UGS_ANS_HitWeaponForChan::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (AGS_Character* Monster = Cast<AGS_Character>(MeshComp->GetOwner()))
	{
		if (AGS_WeaponAxe* Weapon = Cast<AGS_WeaponAxe>(Monster->GetWeaponByIndex(0)))
		{
			Weapon->ServerEnableHit();
		}
	}
}

void UGS_ANS_HitWeaponForChan::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (AGS_Character* Monster = Cast<AGS_Character>(MeshComp->GetOwner()))
	{
		if (AGS_WeaponAxe* Weapon = Cast<AGS_WeaponAxe>(Monster->GetWeaponByIndex(0)))
		{
			Weapon->ServerDisableHit();
		}
	}
}