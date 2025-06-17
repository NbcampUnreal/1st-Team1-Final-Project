// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SwordAuraProjectile.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGS_SwordAuraProjectile::AGS_SwordAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// ↘ 방향
	SlashBoxA = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashBoxA"));
	SlashBoxA->SetupAttachment(RootComponent);
	SlashBoxA->SetBoxExtent(FVector(100.f, 20.f, 100.f));
	SlashBoxA->SetRelativeRotation(FRotator(0.f, 0.f, 45.f));
	SlashBoxA->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// ↙ 방향
	SlashBoxB = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashBoxB"));
	SlashBoxB->SetupAttachment(RootComponent);
	SlashBoxB->SetBoxExtent(FVector(100.f, 20.f, 100.f));
	SlashBoxB->SetRelativeRotation(FRotator(0.f, 0.f, -45.f));
	SlashBoxB->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 중앙 Sphere
	CenterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CenterSphere"));
	CenterSphere->SetupAttachment(RootComponent);
	CenterSphere->SetSphereRadius(75.f);
	CenterSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	ProjectileMovementComponent->InitialSpeed = 500.0f;
	ProjectileMovementComponent->MaxSpeed = 500.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AGS_SwordAuraProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩
	SlashBoxA->OnComponentBeginOverlap.AddDynamic(this, &AGS_SwordAuraProjectile::OnSlashBoxOverlap);
	SlashBoxB->OnComponentBeginOverlap.AddDynamic(this, &AGS_SwordAuraProjectile::OnSlashBoxOverlap);
	CenterSphere->OnComponentBeginOverlap.AddDynamic(this, &AGS_SwordAuraProjectile::OnCenterSphereOverlap);
}

void AGS_SwordAuraProjectile::OnSlashBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		UGameplayStatics::ApplyDamage(OtherActor, BaseDamage * 2.0f, GetInstigatorController(), this, nullptr);
	}
}

void AGS_SwordAuraProjectile::OnCenterSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, GetInstigatorController(), this, nullptr);
	}
}
