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
	
	if (AGS_Monster* Monster = Cast<AGS_Monster>(MeshComp->GetOwner()))
	{
		if (AGS_WeaponSword* Weapon = Cast<AGS_WeaponSword>(Monster->GetWeaponByIndex(0)))
		{
			if (Monster->HasAuthority())
			{
				Weapon->EnableHit();
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
	
	if (AGS_Monster* Monster = Cast<AGS_Monster>(MeshComp->GetOwner()))
	{
		if (AGS_WeaponSword* Weapon = Cast<AGS_WeaponSword>(Monster->GetWeaponByIndex(0)))
		{
			if (Monster->HasAuthority())
			{
				Weapon->DisableHit();
			}
		}
	}
}
