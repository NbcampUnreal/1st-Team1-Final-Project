// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponSword.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"

AGS_WeaponSword::AGS_WeaponSword()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	RootComponent = Mesh;

	HitBox = CreateDefaultSubobject<UBoxComponent>("HitBox");
	HitBox->SetupAttachment(Mesh);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponSword::OnHit);

	OwnerChar = nullptr;
	bReplicates = true;
}


void AGS_WeaponSword::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<AGS_Character>(GetAttachParentActor());
	UE_LOG(LogTemp, Warning, TEXT("Weapon OwnerChar : %s"),*GetNameSafe(OwnerChar));
}


void AGS_WeaponSword::EnableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AGS_WeaponSword::DisableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_WeaponSword::Server_SetHitCollision_Implementation(bool bEnable)
{
	HitBox->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AGS_WeaponSword::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	AGS_Character* Damaged = Cast<AGS_Character>(OtherActor);
	AGS_Character* Attacker = OwnerChar;

	if (!Damaged || !Attacker)
	{
		return;
	}

	UGS_StatComp* DamagedStat = Damaged->GetStatComp();
	if (!DamagedStat) 
	{
		return;	
	}

	float Damage = DamagedStat->CalculateDamage(Attacker, Damaged);
	FDamageEvent DamageEvent;
	Damaged->TakeDamage(Damage, DamageEvent, OwnerChar->GetController(), OwnerChar);

	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
