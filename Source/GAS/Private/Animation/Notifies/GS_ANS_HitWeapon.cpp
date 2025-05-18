// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_HitWeapon.h"
#include "Character/Player/Monster/GS_IronFang.h"
#include "Weapon/Equipable/GS_WeaponSword.h"

void UGS_ANS_HitWeapon::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (AGS_IronFang* IronFang = Cast<AGS_IronFang>(Owner))
	{
		if (AGS_WeaponSword* Weapon = IronFang->GetCurrentWeapon())
		{
			Weapon->EnableHit();
		}
	}
}

void UGS_ANS_HitWeapon::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (AGS_IronFang* IronFang = Cast<AGS_IronFang>(Owner))
	{
		if (AGS_WeaponSword* Weapon = IronFang->GetCurrentWeapon())
		{
			Weapon->DisableHit();
		}
	}
}
