// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Character/Component/GS_StatComp.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"

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

	// 중복 히트 방지
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);

	// 맞은 대상 구분
	EAxeHitTargetType TargetType = DetermineTargetType(OtherActor);

	// HitResult 생성 (Overlap에서는 정확한 히트 포인트가 없을 수 있음)
	FHitResult CorrectHitResult = SweepResult;
	if (!bFromSweep)
	{
		CorrectHitResult.ImpactPoint = GetActorLocation();
		CorrectHitResult.Location = GetActorLocation();
		CorrectHitResult.ImpactNormal = FVector::UpVector;
		CorrectHitResult.Normal = FVector::UpVector;
	}

	Multicast_PlayHitSound(TargetType, CorrectHitResult);
	Multicast_PlayHitVFX(TargetType, CorrectHitResult);

	// PlayHitSound(TargetType, CorrectHitResult);
	// PlayHitVFX(TargetType, CorrectHitResult);
	
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

EAxeHitTargetType AGS_WeaponAxe::DetermineTargetType(AActor* OtherActor) const
{
	if (Cast<AGS_Monster>(OtherActor))
	{
		return EAxeHitTargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		return EAxeHitTargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		return EAxeHitTargetType::Seeker;
	}
	else if (Cast<AGS_Character>(OtherActor))
	{
		return EAxeHitTargetType::Other;
	}
	else
	{
		return EAxeHitTargetType::Structure;
	}
}

void AGS_WeaponAxe::PlayHitSound(EAxeHitTargetType TargetType, const FHitResult& SweepResult)
{
	UAkAudioEvent* SoundEventToPlay = nullptr;

	switch (TargetType)
	{
	case EAxeHitTargetType::Guardian:
	case EAxeHitTargetType::DungeonMonster:
		SoundEventToPlay = HitPawnSoundEvent;
		break;
	case EAxeHitTargetType::Structure:
		SoundEventToPlay = HitStructureSoundEvent;
		break;
	case EAxeHitTargetType::Seeker:
	case EAxeHitTargetType::Other:
		break;
	default:
		break;
	}

	if (SoundEventToPlay && GetWorld())
	{
		UAkGameplayStatics::PostEventAtLocation(
			SoundEventToPlay,
			SweepResult.ImpactPoint,
			FRotator::ZeroRotator,
			GetWorld()
		);
	}
}

void AGS_WeaponAxe::PlayHitVFX(EAxeHitTargetType TargetType, const FHitResult& SweepResult)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (TargetType)
	{
	case EAxeHitTargetType::Guardian:
	case EAxeHitTargetType::DungeonMonster:
		VFXToPlay = HitPawnVFX;
		break;
	case EAxeHitTargetType::Structure:
		VFXToPlay = HitStructureVFX;
		break;
	case EAxeHitTargetType::Seeker:
	case EAxeHitTargetType::Other:
		break;
	default:
		break;
	}

	if (VFXToPlay && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			SweepResult.ImpactPoint,
			SweepResult.ImpactNormal.Rotation(),
			FVector(1.0f),
			true,
			true
		);
	}
}

// 멀티캐스트 함수 구현
bool AGS_WeaponAxe::Multicast_PlayHitSound_Validate(EAxeHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponAxe::Multicast_PlayHitSound_Implementation(EAxeHitTargetType TargetType, const FHitResult& SweepResult)
{
	PlayHitSound(TargetType, SweepResult);
}

bool AGS_WeaponAxe::Multicast_PlayHitVFX_Validate(EAxeHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponAxe::Multicast_PlayHitVFX_Implementation(EAxeHitTargetType TargetType, const FHitResult& SweepResult)
{
	PlayHitVFX(TargetType, SweepResult);
}

void AGS_WeaponAxe::EnableHit()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 히트 액터 목록 초기화 (새로운 공격 시작 시)
	HitActors.Empty();
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
	// 히트 액터 목록 초기화 (새로운 공격 시작 시)
	HitActors.Empty();
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

