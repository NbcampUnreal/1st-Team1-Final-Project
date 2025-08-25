// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponAxe.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Character/Component/GS_StatComp.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Character/F_GS_DamageEvent.h"
#include "ResourceSystem/Aether/GS_AetherExtractor.h"
#include "AI/RTS/GS_RTSController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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
	
	AGS_Character* Damaged = Cast<AGS_Character>(OtherActor);
	AGS_Character* Attacker = OwnerChar;

	//에테르 추출기
	if (!Damaged && Attacker)
	{
		if (AGS_AetherExtractor* AetherExtractor = Cast<AGS_AetherExtractor>(OtherActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("[AGS_WeaponAxe]OnHit is called"));
			float Damage = Attacker->GetStatComp()->GetAttackPower();
			FGS_DamageEvent DamageEvent;
			AetherExtractor->TakeDamageBySeeker(Damage, OwnerChar);
			HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	if (!Damaged || !Attacker || !Damaged->IsEnemy(Attacker))
	{
		// 적이 아닌 대상(벽 등)을 타격한 경우, 기본 VFX만 재생하고 종료
		Multicast_PlayHitVFX(TargetType, CorrectHitResult);
		return;
	}

	// --- 여기서부터는 유효한 적을 타격한 경우 ---
	
	// 1. 기본 VFX는 항상 재생
	Multicast_PlayHitVFX(TargetType, CorrectHitResult);

	// 2. '찬'의 4번째 공격일 경우 추가 효과(사운드, VFX) 재생
	if (AGS_Chan* Chan = Cast<AGS_Chan>(Attacker))
	{
		if (Chan->CurrentComboIndex == 4)
		{
			// 추가 사운드
			Chan->Multicast_OnAttackHit(Chan->CurrentComboIndex);
			
			// 추가 VFX
			if (Chan->FinalAttackHitVFX)
			{
				Multicast_PlaySpecialHitVFX(Chan->FinalAttackHitVFX, CorrectHitResult);
			}
		}
	}
	
	UGS_StatComp* DamagedStat = Damaged->GetStatComp();
	if (!DamagedStat) 
	{
		return;	
	}

	float Damage = DamagedStat->CalculateDamage(Attacker, Damaged);
	FGS_DamageEvent DamageEvent;
	DamageEvent.HitReactType = EHitReactType::Interrupt;
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
		FVector ListenerLocation;
		if (GetListenerLocation(ListenerLocation))
		{
			// RTS 모드와 TPS 모드에 따른 거리 체크
			const bool bRTS = IsRTSMode();
			const float MaxDistance = bRTS ? 10000.0f : 2000.0f; // RTS: 100m, TPS: 20m

			const float DistanceToListener = FVector::Dist(SweepResult.ImpactPoint, ListenerLocation);

			
			if (DistanceToListener <= MaxDistance)
			{
				UAkGameplayStatics::PostEventAtLocation(
					SoundEventToPlay,
					SweepResult.ImpactPoint,
					FRotator::ZeroRotator,
					GetWorld()
				);
			}
		}
		else
		{
			// Fallback: 리스너 위치를 찾지 못할 경우 거리 체크 없이 재생
			UAkGameplayStatics::PostEventAtLocation(
				SoundEventToPlay,
				SweepResult.ImpactPoint,
				FRotator::ZeroRotator,
				GetWorld()
			);
		}
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

void AGS_WeaponAxe::Multicast_PlaySpecialHitVFX_Implementation(UNiagaraSystem* VFXToPlay, const FHitResult& HitResult)
{
	if (VFXToPlay && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			HitResult.ImpactPoint,
			HitResult.ImpactNormal.Rotation(),
			FVector(1.0f),
			true,
			true
		);
	}
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

bool AGS_WeaponAxe::GetListenerLocation(FVector& OutLocation) const
{
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC)
	{
		return false;
	}

	if (AGS_RTSController* RTSController = Cast<AGS_RTSController>(LocalPC))
	{
		if (RTSController->GetViewTarget())
		{
			OutLocation = RTSController->GetViewTarget()->GetActorLocation();
			return true;
		}
	}
	else if (LocalPC->GetPawn())
	{
		OutLocation = LocalPC->GetPawn()->GetActorLocation();
		return true;
	}

	return false;
}

bool AGS_WeaponAxe::IsRTSMode() const
{
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	return LocalPC && Cast<AGS_RTSController>(LocalPC) != nullptr;
}

