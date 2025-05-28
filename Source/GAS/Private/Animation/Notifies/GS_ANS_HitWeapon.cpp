// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_HitWeapon.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Weapon/Equipable/GS_WeaponSword.h"

void UGS_ANS_HitWeapon::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}
	
	if (AGS_Monster* Character = Cast<AGS_Monster>(MeshComp->GetOwner()))
	{
		if (AGS_WeaponSword* Weapon = Cast<AGS_WeaponSword>(Character->GetCurrentWeapon()))
		{
			if (Character->HasAuthority())
			{
				Weapon->EnableHit();
			}
			else
			{
				Weapon->Server_SetHitCollision(true);
			}
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

	if (AGS_Monster* Character = Cast<AGS_Monster>(MeshComp->GetOwner()))
	{
		if (AGS_WeaponSword* Weapon = Cast<AGS_WeaponSword>(Character->GetCurrentWeapon()))
		{
			if (Character->HasAuthority())
			{
				Weapon->DisableHit();
			}
			else
			{
				Weapon->Server_SetHitCollision(false);
			}
		}
	}
}
