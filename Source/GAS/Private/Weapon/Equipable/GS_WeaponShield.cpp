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
	DefenseHitBox->OnComponentEndOverlap.AddDynamic(this, &AGS_WeaponShield::OnDefenseEndOverlap);
}

AGS_Character* AGS_WeaponShield::FindUltimateAttacker(AActor* InActor)
{
	AActor* CurrentActor = InActor;
	AGS_Character* FoundCharacter = nullptr;

	// 최대 10번의 연쇄만 탐색 (무한 루프 방지)
	for (int32 i = 0; i < 10 && CurrentActor != nullptr; ++i)
	{
		// 1. 현재 액터가 AGS_Character인지 확인
		FoundCharacter = Cast<AGS_Character>(CurrentActor);
		if (FoundCharacter)
		{
			// 찾았으면 즉시 반환
			return FoundCharacter;
		}

		// 2. (NEW) 현재 액터의 Instigator가 있는지 확인 (투사체 케이스)
		// GetInstigator()는 APawn*를 반환합니다.
		APawn* InstigatorPawn = CurrentActor->GetInstigator();
		if (InstigatorPawn)
		{
			// Instigator가 Pawn이므로, 바로 AGS_Character로 캐스팅 시도
			FoundCharacter = Cast<AGS_Character>(InstigatorPawn);
			if (FoundCharacter)
			{
				// Instigator가 AGS_Character면 바로 반환
				return FoundCharacter;
			}
			else
			{
				// Instigator가 AGS_Character는 아니지만
				// 다음 탐색을 위해 CurrentActor를 Instigator로 설정
				CurrentActor = InstigatorPawn;
				continue; // 다음 루프 시작
			}
		}

		// 3. (Original) Instigator가 없으면, Owner를 탐색 (무기, 몬스터 콜리전 케이스)
		AActor* OwnerActor = CurrentActor->GetOwner();
		if (OwnerActor)
		{
			// 3a. 소유자가 컨트롤러인지 확인 (몬스터 콜리전 케이스)
			AController* OwnerAsController = Cast<AController>(OwnerActor);
			if (OwnerAsController)
			{
				// 컨트롤러가 빙의한 폰을 다음 탐색 대상으로 지정
				CurrentActor = OwnerAsController->GetPawn();
			}
			else
			{
				// 3b. 소유자가 컨트롤러가 아님 (무기 계층 케이스)
				CurrentActor = OwnerActor;
			}
		}
		else
		{
			// Owner도 없으면 탐색 종료
			CurrentActor = nullptr;
		}
	}

	// 탐색 실패
	return nullptr;
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
	DefenseHitActors.Empty();

	// Tick 비활성화
	SetActorTickEnabled(false);

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AGS_WeaponShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 주기적으로 히트 액터 목록 정리 (메모리 누수 방지)
	static float CleanupTimer = 0.0f;
	CleanupTimer += DeltaTime;
	if (CleanupTimer >= 1.5f) // 1.5초로 단축하여 연속 공격 시 가드 이펙트가 다시 나올 수 있도록 함
	{
		CleanupTimer = 0.0f;
		AttackHitActors.Empty();
		//DefenseHitActors.Empty();
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

	// 2. 슬래시 이펙트 재생 (혈흔 이펙트)
	if (WeaponVFXComponent)
	{
		// 공격자(OwnerChar)의 시커 타입을 직접 전달
		ESeekerAuraType AttackerAuraType = GetSeekerAuraType(OwnerChar);
		WeaponVFXComponent->PlaySlashVFX(CorrectHitResult, AttackerAuraType);
	}

	// 3. 아우라 이펙트 트리거 (가디언이나 몬스터를 타격했을 때)
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

void AGS_WeaponShield::PlayGuardSuccessVFX(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (TargetType)
	{
	case EShieldHitTargetType::Guardian:
	case EShieldHitTargetType::DungeonMonster:
		VFXToPlay = GuardSuccessPawnVFX;
		break;
	case EShieldHitTargetType::Structure:
		VFXToPlay = GuardSuccessStructureVFX;
		break;
	case EShieldHitTargetType::Seeker:
	case EShieldHitTargetType::Other:
		break;
	default:
		break;
	}

	if (VFXToPlay && GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_WeaponShield::PlayGuardSuccessVFX - Playing common guard VFX: %s"), *VFXToPlay->GetName());
		// 방패 중앙에서 이펙트 재생
		FVector ShieldCenter = ShieldMeshComponent->GetComponentLocation();
		FRotator ShieldRotation = ShieldMeshComponent->GetComponentRotation();
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFXToPlay,
			ShieldCenter,
			ShieldRotation,
			FVector(1.2f), // 방패 이펙트는 약간 크게
			true,
			true
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_WeaponShield::PlayGuardSuccessVFX - Common guard VFX is not set or World is invalid."));
	}

	// WeaponVFXComponent를 통한 시커별 개별 가드 이펙트도 재생
	if (WeaponVFXComponent && OwnerChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_WeaponShield::PlayGuardSuccessVFX - Calling WeaponVFXComponent->PlayGuardSuccessVFX"));
		ESeekerAuraType DefenderAuraType = GetSeekerAuraType(OwnerChar);
		WeaponVFXComponent->PlayGuardSuccessVFX(SweepResult, DefenderAuraType);
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

bool AGS_WeaponShield::Multicast_PlayGuardSuccessVFX_Validate(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponShield::Multicast_PlayGuardSuccessVFX_Implementation(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	PlayGuardSuccessVFX(TargetType, SweepResult);
}

void AGS_WeaponShield::PlayGuardSuccessSound(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	UAkAudioEvent* SoundEventToPlay = nullptr;

	switch (TargetType)
	{
	case EShieldHitTargetType::Guardian:
	case EShieldHitTargetType::DungeonMonster:
		SoundEventToPlay = GuardSuccessPawnSoundEvent;
		break;
	case EShieldHitTargetType::Structure:
		SoundEventToPlay = GuardSuccessStructureSoundEvent;
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

bool AGS_WeaponShield::Multicast_PlayGuardSuccessSound_Validate(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	return true;
}

void AGS_WeaponShield::Multicast_PlayGuardSuccessSound_Implementation(EShieldHitTargetType TargetType, const FHitResult& SweepResult)
{
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition())
	{
		return;
	}

	PlayGuardSuccessSound(TargetType, SweepResult);
}

void AGS_WeaponShield::EnableAttackHit()
{
	// 레벨 전환 중인 경우 안전하게 종료
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// AttackHitBox 안전하게 활성화
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
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
	// 레벨 전환 중인 경우 안전하게 종료
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// AttackHitBox 안전하게 비활성화
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// DefenseHitBox 상태를 안전하게 확인하여 Tick 상태 결정
	bool bShouldDisableTick = true;
	if (DefenseHitBox && IsValid(DefenseHitBox) && !DefenseHitBox->IsBeingDestroyed())
	{
		if (DefenseHitBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			bShouldDisableTick = false;
		}
	}

	// 모든 콜리전이 비활성화된 경우에만 Tick 비활성화
	if (bShouldDisableTick)
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

	// 추가 안전성 검사: 액터와 컴포넌트 유효성 확인
	if (!IsValid(this) || IsActorBeingDestroyed())
	{
		return;
	}
	
	// AttackHitBox 안전하게 비활성화
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// DefenseHitBox 상태를 안전하게 확인하여 Tick 상태 결정
	bool bShouldDisableTick = true;
	if (DefenseHitBox && IsValid(DefenseHitBox) && !DefenseHitBox->IsBeingDestroyed())
	{
		if (DefenseHitBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			bShouldDisableTick = false;
		}
	}

	// 모든 콜리전이 비활성화된 경우에만 Tick 비활성화
	if (bShouldDisableTick)
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

	// 추가 안전성 검사: 액터와 컴포넌트 유효성 확인
	if (!IsValid(this) || IsActorBeingDestroyed())
	{
		return;
	}
	
	// AttackHitBox 안전하게 활성화
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
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
	if (UWorld* World = GetWorld(); World && !World->bIsTearingDown && IsValid(World))
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (&TimerManager)
		{
			TimerManager.SetTimer(SafetyTimerHandle, this, &AGS_WeaponShield::DisableAttackHit, 0.2f, false);
		}
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

	// 추가 안전성 검사: 액터와 컴포넌트 유효성 확인
	if (!IsValid(this) || IsActorBeingDestroyed())
	{
		return;
	}

	// 컴포넌트별 개별 유효성 검사와 함께 비활성화
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// DefenseHitBox도 동시에 체크하여 Tick 상태 결정
	bool bShouldDisableTick = true;
	if (DefenseHitBox && IsValid(DefenseHitBox) && !DefenseHitBox->IsBeingDestroyed())
	{
		if (DefenseHitBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			bShouldDisableTick = false;
		}
	}

	// 모든 콜리전이 비활성화된 경우에만 Tick 비활성화
	if (bShouldDisableTick)
	{
		SetActorTickEnabled(false);
	}

	// 안전장치 타이머 정리
	ClearSafetyTimer();
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

	// 충돌한 컴포넌트가 방어가 가능한 공격인지 확인
	if (!OtherComp->ComponentHasTag("DEFENSIBLE_ATTACK"))
	{
		UE_LOG(LogTemp, Warning, TEXT("[찬 방어] DEFENSIBLE_ATTACK 태그 없음: %s"), *OtherComp->GetName());
		return;
	}
	
	// OwnerChar 유효성 확인 (레벨 전환 시 null일 수 있음.)
	if (!IsOwnerCharValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[찬 방어] OwnerChar 유효하지 않음"));
		return;
	}
	
	// 실제 공격자(캐릭터)를 찾기. OtherActor는 무기일 수 있음.
	AActor* AttackerActor = FindUltimateAttacker(OtherActor);
	
	// 중복 방어 히트 방지
	if (DefenseHitActors.Contains(AttackerActor))
	{
		return;
	}
	DefenseHitActors.Add(AttackerActor);

	// 맞은 대상 구분
	EShieldHitTargetType TargetType = DetermineTargetType(AttackerActor);

	FHitResult CorrectHitResult = CreateCorrectHitResult(SweepResult, bFromSweep);

	AGS_Character* Attacker = Cast<AGS_Character>(AttackerActor); // 공격자
	AGS_Character* Defender = OwnerChar; // 방어자
	if (!Attacker || !Defender || !Attacker->IsEnemy(Defender))
	{
		return;
	}
	// 찬이 방어 상태일 때만 가드 성공으로 인정
	AGS_Chan* Chan = Cast<AGS_Chan>(Defender);
	if (!Chan || !Chan->bIsDefending)
	{
		return;
	}
	// === 가드 성공 이펙트 재생 ===
	// 방어 성공 시 방패에서 이펙트와 사운드 재생
	Multicast_PlayGuardSuccessVFX(TargetType, CorrectHitResult);
	Multicast_PlayGuardSuccessSound(TargetType, CorrectHitResult);

	// 찬 전용 추가 방어 사운드 (시커 오디오 컴포넌트)
	if (UGS_SeekerAudioComponent* SeekerAudio = Chan->GetComponentByClass<UGS_SeekerAudioComponent>())
	{
		SeekerAudio->PlayDefenseSound();
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
			World->GetTimerManager().SetTimer(ReEnableCollisionHandle, [this, OtherComp, AttackerActor]()
			{
				if (OtherComp && IsValid(OtherComp))
				{
					OtherComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				}
				//DefenseHitActors.Remove(FindUltimateAttacker(OtherActor));
				DefenseHitActors.Remove(AttackerActor);
			}, 0.1f, false);
		}
	}
}

void AGS_WeaponShield::OnDefenseEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	
	// 레벨 전환 시 null 참조 방지
	if (!IsValidForLevelTransition() || !OtherActor)
	{
		return;
	}
	
	// 실제 공격자(캐릭터)를 찾기
	AActor* AttackerActor = FindUltimateAttacker(OtherActor);
	
	// 방어용 히트 액터 목록에서 제거하여 다음 공격 시 가드 이펙트가 다시 나올 수 있도록 함
	DefenseHitActors.Remove(AttackerActor);
	UE_LOG(LogTemp, Warning, TEXT("[찬 방어] 중복 해제 성공"));
}

void AGS_WeaponShield::EnableDefenseHit()
{
	// 레벨 전환 중인 경우 안전하게 종료
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// DefenseHitBox 안전하게 활성화
	if (DefenseHitBox && IsValid(DefenseHitBox) && !DefenseHitBox->IsBeingDestroyed())
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	// 방어용 히트 액터 목록 초기화 (새로운 방어 시작 시)
	DefenseHitActors.Empty();
	
	SetActorTickEnabled(true);
}

void AGS_WeaponShield::DisableDefenseHit()
{
	// 레벨 전환 중인 경우 안전하게 종료
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// DefenseHitBox 안전하게 비활성화
	if (DefenseHitBox && IsValid(DefenseHitBox) && !DefenseHitBox->IsBeingDestroyed())
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// AttackHitBox 상태를 안전하게 확인하여 Tick 상태 결정
	bool bShouldDisableTick = true;
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
	{
		if (AttackHitBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			bShouldDisableTick = false;
		}
	}

	// 모든 콜리전이 비활성화된 경우에만 Tick 비활성화
	if (bShouldDisableTick)
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

	// 추가 안전성 검사: 액터와 컴포넌트 유효성 확인
	if (!IsValid(this) || IsActorBeingDestroyed())
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

	// 추가 안전성 검사: 액터와 컴포넌트 유효성 확인
	if (!IsValid(this) || IsActorBeingDestroyed())
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
	// 레벨 전환 중인 경우 안전하게 종료
	if (!IsValidForLevelTransition())
	{
		return;
	}

	// AttackHitBox 안전하게 비활성화
	if (AttackHitBox && IsValid(AttackHitBox) && !AttackHitBox->IsBeingDestroyed())
	{
		AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// DefenseHitBox 안전하게 비활성화
	if (DefenseHitBox && IsValid(DefenseHitBox) && !DefenseHitBox->IsBeingDestroyed())
	{
		DefenseHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}