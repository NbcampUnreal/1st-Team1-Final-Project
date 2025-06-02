// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Character/GS_Character.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Character/Component/GS_StatComp.h"

// Sets default values
AGS_WeaponAxe::AGS_WeaponAxe()
{
	bReplicates = true;
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	OwnerChar = nullptr;

	AxeMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AxeMeshComponent"));
	RootComponent = AxeMeshComponent;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Greataxe_01/SKM_Greataxe_01.SKM_Greataxe_01"));
	if (MeshAsset.Succeeded())
	{
		AxeMeshComponent->SetSkeletalMesh(MeshAsset.Object);
	}

	HitBox = CreateDefaultSubobject<UBoxComponent>("HitBox");
	HitBox->SetupAttachment(AxeMeshComponent);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponAxe::OnHit);
}

void AGS_WeaponAxe::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	
	AGS_Character* Damaged = Cast<AGS_Character>(OtherActor);
	AGS_Character* Attacker = OwnerChar;

	if (!Damaged || !Attacker || !Damaged->IsEnemy(Attacker))
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

void AGS_WeaponAxe::EnableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AGS_WeaponAxe::DisableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_WeaponAxe::ServerDisableHit_Implementation()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGS_WeaponAxe::ServerEnableHit_Implementation()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called when the game starts or when spawned
void AGS_WeaponAxe::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<AGS_Character>(GetOwner());
}

// Called every frame
void AGS_WeaponAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

