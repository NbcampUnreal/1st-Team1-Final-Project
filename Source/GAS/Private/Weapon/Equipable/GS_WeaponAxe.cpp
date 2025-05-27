// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Equipable/GS_WeaponAxe.h"

AGS_WeaponAxe::AGS_WeaponAxe()
{
	// 도끼 메시 생성
	AxeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AxeMesh"));
	RootComponent = AxeMesh;

	// 히트박스를 도끼 메시에 붙임
	HitBox->SetupAttachment(AxeMesh);

	// 도끼 메시 에셋 로드
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Greataxe_01/SKM_Greataxe_01.SKM_Greataxe_01"));
	if (MeshAsset.Succeeded())
	{
		AxeMesh->SetSkeletalMesh(MeshAsset.Object);
	}
}