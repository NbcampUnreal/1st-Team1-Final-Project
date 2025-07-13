// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponEquipable.h"

AGS_WeaponEquipable::AGS_WeaponEquipable()
{
}

void AGS_WeaponEquipable::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Set Server option
	SetReplicateMovement(true); // Replicate Actor Rotation & Transition
}
