// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGS_WeaponProjectile::AGS_WeaponProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AGS_WeaponProjectile::OnHit);
	CollisionComponent->SetNotifyRigidBodyCollision(true);

	ProjectileMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	
}

void AGS_WeaponProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
}


void AGS_WeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->InitialSpeed;
	}
}

void AGS_WeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
