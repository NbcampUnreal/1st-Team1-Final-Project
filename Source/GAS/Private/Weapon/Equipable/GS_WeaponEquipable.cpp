// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponEquipable.h"

AGS_WeaponEquipable::AGS_WeaponEquipable()
{
	OwnerCharacter = nullptr;
}

void AGS_WeaponEquipable::SetOwningCharacter(AGS_Seeker* Character)
{
	OwnerCharacter = Character;
}

void AGS_WeaponEquipable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Set Server option
	SetReplicateMovement(true); // Replicate Actor Rotation & Transition
}
