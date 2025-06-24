// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/GS_ANS_HitWeaponForChan.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Weapon/Equipable/GS_WeaponSword.h"

void UGS_ANS_HitWeaponForChan::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		ECharacterType CharacterType = Seeker->GetCharacterType();
		if (CharacterType == ECharacterType::Chan)
		{
			if (AGS_WeaponAxe* Weapon = Cast<AGS_WeaponAxe>(Seeker->GetWeaponByIndex(0)))
			{
				UE_LOG(LogTemp, Warning, TEXT("NotifyBegin, ServerEnableHit, Chan"));
				Weapon->ServerEnableHit();
			}
		}
		else if (CharacterType == ECharacterType::Ares)
		{
			if (AGS_WeaponSword* Weapon = Cast<AGS_WeaponSword>(Seeker->GetWeaponByIndex(0)))
			{
				UE_LOG(LogTemp, Warning, TEXT("NotifyBegin, ServerEnableHit, Ares"));
				Weapon->ServerEnableHit();
			}
		}
	}	
}

void UGS_ANS_HitWeaponForChan::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(MeshComp->GetOwner()))
	{
		ECharacterType CharacterType = Seeker->GetCharacterType();
		if (CharacterType == ECharacterType::Chan)
		{
			if (AGS_WeaponAxe* Weapon = Cast<AGS_WeaponAxe>(Seeker->GetWeaponByIndex(0)))
			{
				Weapon->ServerDisableHit();
				/*if (Attacker->HasAuthority())
				{
					Weapon->ServerDisableHit();
				}*/
			}
		}
		else if (CharacterType == ECharacterType::Ares)
		{
			if (AGS_WeaponSword* Weapon = Cast<AGS_WeaponSword>(Seeker->GetWeaponByIndex(0)))
			{
				Weapon->ServerDisableHit();
				/*if (Attacker->HasAuthority())
				{
					Weapon->ServerDisableHit();
				}*/
			}
		}
	}
}