// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponWand.h"
#include "Character/GS_Character.h"

AGS_WeaponWand::AGS_WeaponWand()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	RootComponent = Mesh;
}

void AGS_WeaponWand::BeginPlay()
{
	Super::BeginPlay();
}
