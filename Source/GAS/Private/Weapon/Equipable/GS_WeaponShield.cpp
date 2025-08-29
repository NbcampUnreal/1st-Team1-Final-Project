// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Equipable/GS_WeaponShield.h"
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
#include "Sound/GS_SeekerAudioComponent.h"


// Sets default values
AGS_WeaponShield::AGS_WeaponShield()
{
	// server
	bReplicates = true;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set SKM
	ShieldMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShieldMeshComponent"));
	RootComponent = ShieldMeshComponent;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Shield_02/SKM_Shield_02_L.SKM_Shield_02_L"));
	
	if (MeshAsset.Succeeded())
	{
		ShieldMeshComponent->SetSkeletalMesh(MeshAsset.Object);
	}

	// 공격용 콜리전 (기존 HitBox)
	AttackHitBox = CreateDefaultSubobject<UBoxComponent>("AttackHitBox");
	AttackHitBox->SetupAttachment(ShieldMeshComponent);
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackHitBox->SetGenerateOverlapEvents(true);
	
	// 공격용 콜리전 크기 설정
	AttackHitBox->SetBoxExtent(FVector(80.0f, 120.0f, 150.0f));
	AttackHitBox->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
	AttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponShield::OnAttackHit);
	
	// 방어용 콜리전
	DefenseHitBox = CreateDefaultSubobject<UBoxComponent>("DefenseHitBox");
	DefenseHitBox->SetupAttachment(ShieldMeshComponent);
	DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DefenseHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DefenseHitBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	DefenseHitBox->SetGenerateOverlapEvents(true);
	
	// 방어용 콜리전 크기 설정 (방패 전체 영역)
	DefenseHitBox->SetBoxExtent(FVector(100.0f, 140.0f, 170.0f));
	DefenseHitBox->SetRelativeLocation(FVector(30.0f, 0.0f, 0.0f));
	DefenseHitBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponShield::OnDefenseHit);
}

void AGS_WeaponShield::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AGS_WeaponShield::BeginPlay()
{
	Super::BeginPlay();
	OwnerChar = Cast<AGS_Character>(GetOwner());
	
	if (AttackHitBox)
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (DefenseHitBox)
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AGS_WeaponShield::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 레벨 전환 시 타이머 정리
	ClearSafetyTimer();

	// 콜리전 비활성화로 추가적인 이벤트 방지
	DisableAllCollisions();

	// 히트 액터 목록 정리
	AttackHitActors.Empty();

	// Tick 비활성화
	SetActorTickEnabled(false);

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGS_WeaponShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 주기적으로 AttackHitActors 정리 (메모리 누수 방지)
	static float CleanupTimer = 0.0f;
	CleanupTimer += DeltaTime;
	if (CleanupTimer >= 5.0f)
	{
		CleanupTimer = 0.0f;
		AttackHitActors.Empty();
	}
}

void AGS_WeaponShield::OnAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// 중복 히트 방지 (공격용)
	if (!OtherActor || OtherActor == this || AttackHitActors.Contains(OtherActor))
	{
		return;
	}

	AttackHitActors.Add(OtherActor);

	// OwnerChar 유효성 확인 (레벨 전환 시 null일 수 있음)
	if (!IsOwnerCharValid())
	{
		return;
	}

	// 맞은 대상 구분
	EShieldHitTargetType TargetType = DetermineTargetType(OtherActor);

	// HitResult 생성 (Overlap에서는 정확한 히트 포인트가 없을 수 있음)
	FHitResult CorrectHitResult = CreateCorrectHitResult(SweepResult, bFromSweep);

	Multicast_PlayHitSound(TargetType, CorrectHitResult);
	
	AGS_Character* Damaged = Cast<AGS_Character>(OtherActor);
	AGS_Character* Attacker = OwnerChar;

	//에테르 추출기
	if (!Damaged && Attacker)
	{
		if (AGS_AetherExtractor* AetherExtractor = Cast<AGS_AetherExtractor>(OtherActor))
		{
			float Damage = Attacker->GetStatComp()->GetAttackPower();
			FGS_DamageEvent DamageEvent;
			AetherExtractor->TakeDamageBySeeker(Damage, OwnerChar);
			AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	// 2. 아우라 이펙트 트리거 (가디언이나 몬스터를 타격했을 때)
	TriggerHitAuraOnHit(Damaged);

	// 3. '찬'의 3번째 공격일 경우 추가 효과(사운드, VFX) 재생
	if (AGS_Chan* Chan = Cast<AGS_Chan>(Attacker))
	{
		if (Chan->CurrentComboIndex == 3)
		{
			Chan->Multicast_OnAttackHit(Chan->CurrentComboIndex);
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
	
	// 공격 후 콜리전 비활성화 (지속 데미지 방지)
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

EShieldHitTargetType AGS_WeaponShield::DetermineTargetType(AActor* OtherActor) const
{
	if (Cast<AGS_Monster>(OtherActor))
	{
		return EShieldHitTargetType::DungeonMonster;
	}
	else if (Cast<AGS_Guardian>(OtherActor))
	{
		return EShieldHitTargetType::Guardian;
	}
	else if (Cast<AGS_Seeker>(OtherActor))
	{
		return EShieldHitTargetType::Seeker;
	}
	else if (Cast<AGS_Character>(OtherActor))
	{
		return EShieldHitTargetType::Other;
	}
	else
	{
		return EShieldHitTargetType::Structure;
	}
}

void AGS_WeaponShield::PlayHitSound(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	UAkAudioEvent* SoundEventToPlay = nullptr;

	switch (TargetType)
	{
	case EShieldHitTargetType::Guardian:
	case EShieldHitTargetType::DungeonMonster:
		SoundEventToPlay = HitPawnSoundEvent;
		break;
	case EShieldHitTargetType::Structure:
		SoundEventToPlay = HitStructureSoundEvent;
		break;
	case EShieldHitTargetType::Seeker:
	case EShieldHitTargetType::Other:
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

void AGS_WeaponShield::PlayHitVFX(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (TargetType)
	{
	case EShieldHitTargetType::Guardian:
	case EShieldHitTargetType::DungeonMonster:
		VFXToPlay = HitPawnVFX;
		break;
	case EShieldHitTargetType::Structure:
		VFXToPlay = HitStructureVFX;
		break;
	case EShieldHitTargetType::Seeker:
	case EShieldHitTargetType::Other:
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
bool AGS_WeaponShield::Multicast_PlayHitSound_Validate(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponShield::Multicast_PlayHitSound_Implementation(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	PlayHitSound(TargetType, SweepResult);
}

bool AGS_WeaponShield::Multicast_PlayHitVFX_Validate(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponShield::Multicast_PlayHitVFX_Implementation(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	PlayHitVFX(TargetType, SweepResult);
}

void AGS_WeaponShield::Multicast_PlaySpecialHitVFX_Implementation(UNiagaraSystem* VFXToPlay, const FHitResult& HitResult)
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

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

void AGS_WeaponShield::EnableAttackHit()
{
	if (AttackHitBox && IsValid(AttackHitBox))
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	// 히트 액터 목록 초기화 (새로운 공격 시작 시)
	AttackHitActors.Empty();
	
	// 콜리전 활성화 시에만 Tick 활성화
	SetActorTickEnabled(true);
}

void AGS_WeaponShield::DisableAttackHit()
{
	if (AttackHitBox && IsValid(AttackHitBox))
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (DefenseHitBox && IsValid(DefenseHitBox) && 
		DefenseHitBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		SetActorTickEnabled(false);
	}
}

void AGS_WeaponShield::ServerDisableAttackHit_Implementation()
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}
	
	if (AttackHitBox && IsValid(AttackHitBox))
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	if (DefenseHitBox && IsValid(DefenseHitBox) && 
		DefenseHitBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		SetActorTickEnabled(false);
	}
}

void AGS_WeaponShield::ServerEnableAttackHit_Implementation()
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}
	
	if (AttackHitBox && IsValid(AttackHitBox))
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	// 히트 액터 목록 초기화 (새로운 공격 시작 시)
	AttackHitActors.Empty();
	
	// 콜리전 활성화 시에만 Tick 활성화
	SetActorTickEnabled(true);
		
	// 안전장치: 3초 후에 자동으로 비활성화 (AnimNotify가 실행되지 않을 경우 대비)
	ClearSafetyTimer();

	// 레벨 전환 중이 아닌 경우에만 타이머 설정
	if (UWorld* World = GetWorld(); World && !World->bIsTearingDown)
	{
		World->GetTimerManager().SetTimer(SafetyTimerHandle, this, &AGS_WeaponShield::DisableAttackHit, 3.0f, false);
	}
}

void AGS_WeaponShield::ServerEnableHit_Implementation()
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	ServerEnableAttackHit();
}

void AGS_WeaponShield::ServerDisableHit_Implementation()
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	ServerDisableAttackHit();
}

void AGS_WeaponShield::OnDefenseHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// 기본 유효성 검사
	if (!OtherActor || OtherActor == this || !OtherComp)
	{
		return;
	}

	// 충돌한 컴포넌트가 몬스터의 무기/공격 콜리전인지 확인
	FString ComponentName = OtherComp->GetName();
	if (!ComponentName.Contains(TEXT("Bite")) && 
		!ComponentName.Contains(TEXT("Weapon")) && 
		!ComponentName.Contains(TEXT("Attack")) &&
		!ComponentName.Contains(TEXT("Claw")) &&
		!ComponentName.Contains(TEXT("Fang")))
	{
		return;
	}

	// OwnerChar 유효성 확인 (레벨 전환 시 null일 수 있음)
	if (!IsOwnerCharValid())
	{
		return;
	}

	// 맞은 대상 구분
	EShieldHitTargetType TargetType = DetermineTargetType(OtherActor);

	FHitResult CorrectHitResult = CreateCorrectHitResult(SweepResult, bFromSweep);

	// 방어 사운드 재생
	Multicast_PlayHitSound(TargetType, CorrectHitResult);
	
	AGS_Character* Attacker = Cast<AGS_Character>(OtherActor); // 공격자
	AGS_Character* Defender = OwnerChar; // 방어자

	if (!Attacker || !Defender || !Attacker->IsEnemy(Defender))
	{
		// 적이 아닌 대상을 막은 경우
		Multicast_PlayHitVFX(TargetType, CorrectHitResult);
		return;
	}

	Multicast_PlayHitVFX(TargetType, CorrectHitResult);

	if (AGS_Chan* Chan = Cast<AGS_Chan>(Defender))
	{
		if (UGS_SeekerAudioComponent* SeekerAudio = Chan->GetComponentByClass<UGS_SeekerAudioComponent>())
		{
			SeekerAudio->PlayDefenseSound();
		}
	}
	
	// 방어 성공 시 몬스터 공격 콜리전을 비활성화하여 데미지 전달 방지
	if (OtherComp)
	{
		// 일시적으로 공격 콜리전 비활성화
		OtherComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// 0.1초 후 콜리전 재활성화 (null 참조 방지)
		if (UWorld* World = GetWorld())
		{
			FTimerHandle ReEnableCollisionHandle;
			World->GetTimerManager().SetTimer(ReEnableCollisionHandle, [OtherComp]()
			{
				if (OtherComp && IsValid(OtherComp))
				{
					OtherComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				}
			}, 0.1f, false);
		}
	}
}

void AGS_WeaponShield::EnableDefenseHit()
{
	if (DefenseHitBox && IsValid(DefenseHitBox))
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	SetActorTickEnabled(true);
}

void AGS_WeaponShield::DisableDefenseHit()
{
	if (DefenseHitBox && IsValid(DefenseHitBox))
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// 콜리전 비활성화 시 Tick 비활성화 (공격 콜리전도 체크)
	if (AttackHitBox && IsValid(AttackHitBox) && 
		AttackHitBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		SetActorTickEnabled(false);
	}
}

void AGS_WeaponShield::ServerEnableDefenseHit_Implementation()
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	EnableDefenseHit();
}

void AGS_WeaponShield::ServerDisableDefenseHit_Implementation()
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	DisableDefenseHit();
}

bool AGS_WeaponShield::GetListenerLocation(FVector& OutLocation) const
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

bool AGS_WeaponShield::IsRTSMode() const
{
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	return LocalPC && Cast<AGS_RTSController>(LocalPC) != nullptr;
}

// ==============
// 헬퍼 함수 구현
// ==============

void AGS_WeaponShield::DisableAllCollisions()
{
	if (AttackHitBox && IsValid(AttackHitBox))
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (DefenseHitBox && IsValid(DefenseHitBox))
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}