// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Equipable/GS_WeaponSword.h"

AGS_WeaponSword::AGS_WeaponSword()
{
	// 검 메시 생성
	SwordMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SwordMesh"));
	RootComponent = SwordMesh;

	// 히트박스를 검 메시에 붙임
	HitBox->SetupAttachment(SwordMesh);

	// 검 메시 에셋 로드
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Sword_01/SKM_Sword_01.SKM_Sword_01"));
	if (MeshAsset.Succeeded())
	{
		SwordMesh->SetSkeletalMesh(MeshAsset.Object);
	}
}