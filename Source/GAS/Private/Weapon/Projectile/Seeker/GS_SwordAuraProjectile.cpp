// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SwordAuraProjectile.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGS_SwordAuraProjectile::AGS_SwordAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	// SlashBox 부착
	SlashBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashBoxA"));
	SlashBox->SetupAttachment(CollisionComponent);
	SlashBox->SetBoxExtent(FVector(100.f, 20.f, 100.f));
	SlashBox->SetCollisionProfileName(TEXT("Arrow"));

	ProjectileMovementComponent->InitialSpeed = 4000.0f;
	ProjectileMovementComponent->MaxSpeed = 4000.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AGS_SwordAuraProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_SwordAuraProjectile, EffectType);
}

void AGS_SwordAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	Multicast_StartSwordSlashVFX();
	// 오버랩 이벤트 바인딩
	SlashBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_SwordAuraProjectile::OnSlashBoxOverlap);	

	GetWorld()->GetTimerManager().SetTimer(DestorySwordAuraHandle, this, &AGS_SwordAuraProjectile::DestroySwordAura, SwordAuraLifetime, false);
}

void AGS_SwordAuraProjectile::OnSlashBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		UGameplayStatics::ApplyDamage(OtherActor, BaseDamage * 2.0f, GetInstigatorController(), this, nullptr);
	}
}

void AGS_SwordAuraProjectile::DestroySwordAura()
{
	Destroy();
}



void AGS_SwordAuraProjectile::StartSwordSlashVFX()
{

}


void AGS_SwordAuraProjectile::StopSwordSlashVFX()
{

}


void AGS_SwordAuraProjectile::Multicast_StartSwordSlashVFX_Implementation()
{
	if (!SlashBox)
	{
		return;
	}

	UNiagaraSystem* SelectedVFX = (EffectType == ESwordAuraEffectType::Left) ? LeftSlashVFX : RightSlashVFX;
	UE_LOG(LogTemp, Warning, TEXT("SelectedVFX : %s"), *SelectedVFX->GetName());
	if (!SelectedVFX)
	{
		return;
	}

	FVector LocalPos = FVector::ZeroVector; // 부모 위치 기준 (붙는 지점 기준)
	FRotator LocalRot = FRotator::ZeroRotator; // 붙는 지점 기준 회전

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SelectedVFX,
		SlashBox,
		NAME_None,
		LocalPos,
		LocalRot, //Projectile 회전값 
		EAttachLocation::KeepRelativeOffset,//월드 기준
		true
	);
}