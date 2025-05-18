// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_ShadowFang.h"
#include "Weapon/Equipable/GS_WeaponSword.h"

AGS_ShadowFang::AGS_ShadowFang()
{
}

void AGS_ShadowFang::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		FName WeaponSocket(TEXT("WeaponSocket"));
		CurrentWeapon = GetWorld()->SpawnActor<AGS_WeaponSword>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
		}
	}
}
