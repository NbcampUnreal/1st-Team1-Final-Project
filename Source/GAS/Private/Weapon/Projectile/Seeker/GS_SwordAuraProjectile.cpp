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
	
	// 오버랩 이벤트 바인딩
	SlashBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_SwordAuraProjectile::OnSlashBoxOverlap);	

	GetWorld()->GetTimerManager().SetTimer(DestorySwordAuraHandle, this, &AGS_SwordAuraProjectile::DestroySwordAura, SwordAuraLifetime, false);
}

void AGS_SwordAuraProjectile::OnSlashBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !HitActors.Contains(OtherActor))
	{
		HitActors.Add(OtherActor);
		
		// 데미지 적용
		UGameplayStatics::ApplyDamage(OtherActor, BaseDamage * 2.0f, GetInstigatorController(), this, nullptr);
		
		// 타격 위치 계산 (히트된 액터의 중심 위치 사용)
		FVector HitLocation = OtherActor->GetActorLocation();
		
		// 타격 VFX 재생 (멀티캐스트)
		if (HasAuthority())
		{
			Multicast_PlayHitVFX(HitLocation);
		}
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

	// VFX 타입 선택
	UNiagaraSystem* SelectedVFX = nullptr;
	switch (EffectType)
	{
	case ESwordAuraEffectType::LeftNormal:
		SelectedVFX = LeftNormalSlashVFX;
		break;
	case ESwordAuraEffectType::RightNormal:
		SelectedVFX = RightNormalSlashVFX;
		break;
	case ESwordAuraEffectType::LeftBuff:
		SelectedVFX = LeftBuffSlashVFX;
		break;
	case ESwordAuraEffectType::RightBuff:
		SelectedVFX = RightBuffSlashVFX;
		break;
	}
	
	if (!SelectedVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwordAuraProjectile: SelectedVFX is null"));
		return;
	}

	// VFX 컴포넌트 생성 및 부착
	FVector LocalPos = FVector::ZeroVector;
	FRotator LocalRot = FRotator::ZeroRotator;

	SlashVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SelectedVFX,
		SlashBox,
		NAME_None,
		LocalPos,
		LocalRot,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (SlashVFXComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("SwordAuraProjectile: Slash VFX Component created successfully"));
	}
}

void AGS_SwordAuraProjectile::Multicast_PlayHitVFX_Implementation(const FVector& HitLocation)
{
	// 궁극기 활성화 상태에 따라 Hit VFX 선택
	bool bIsBuffed = (EffectType == ESwordAuraEffectType::LeftBuff || EffectType == ESwordAuraEffectType::RightBuff);
	UNiagaraSystem* SelectedHitVFX = bIsBuffed ? BuffHitVFX : NormalHitVFX;

	if (!SelectedHitVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwordAuraProjectile: Hit VFX is null"));
	}
	else
	{
		// 타격 이펙트 스폰 (월드 스페이스)
		UNiagaraComponent* HitVFXComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SelectedHitVFX,
			HitLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		if (HitVFXComp)
		{
			UE_LOG(LogTemp, Log, TEXT("SwordAuraProjectile: Hit VFX spawned at location: %s"), *HitLocation.ToString());
		}
	}

	// 혈흔 이펙트 스폰 (버프 상태에 따라 다른 혈흔 선택)
	UNiagaraSystem* SelectedBloodVFX = bIsBuffed ? BuffBloodSplatterVFX : NormalBloodSplatterVFX;
	
	if (SelectedBloodVFX)
	{
		UNiagaraComponent* BloodVFXComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SelectedBloodVFX,
			HitLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		if (BloodVFXComp)
		{
			FString BloodType = bIsBuffed ? TEXT("Buff") : TEXT("Normal");
			UE_LOG(LogTemp, Log, TEXT("SwordAuraProjectile: %s Blood Splatter VFX spawned at location: %s"), 
				*BloodType, *HitLocation.ToString());
		}
	}
	else
	{
		FString BloodType = bIsBuffed ? TEXT("Buff") : TEXT("Normal");
		UE_LOG(LogTemp, Warning, TEXT("SwordAuraProjectile: %s Blood Splatter VFX is null"), *BloodType);
	}
}