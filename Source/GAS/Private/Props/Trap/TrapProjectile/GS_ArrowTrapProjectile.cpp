#include "Props/Trap/TrapProjectile/GS_ArrowTrapProjectile.h"
#include "Components/SphereComponent.h"
#include "Character/Player/GS_Player.h"
#include "Character/GS_Character.h"
#include "Props/Trap/TrapProjectile/GS_TrapVisualProjectile.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Props/Trap/TrapProjectile/GS_ProjectilePoolComp.h"
#include "Net/UnrealNetwork.h"
#include "AkGameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AGS_ArrowTrapProjectile::AGS_ArrowTrapProjectile()
{
	// 기본값 초기화
	ImpactSoundEvent = nullptr;
	PlayerHitSoundEvent = nullptr;
	ImpactVFX = nullptr;
	PlayerHitVFX = nullptr;
}

void AGS_ArrowTrapProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(4.0f);
	ProjectileMesh->SetSimulatePhysics(false);
	CollisionComponent->SetSimulatePhysics(false);
}

void AGS_ArrowTrapProjectile::Init(AGS_NonTrigTrapBase* InTrap)
{
	OwningTrap = InTrap;
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AGS_ArrowTrapProjectile::OnBeginOverlap);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGS_ArrowTrapProjectile::OnBeginOverlap);
		CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel3);
	}
}

void AGS_ArrowTrapProjectile::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this)
	{
		return;
	}

	// 히트 타입 결정
	EArrowHitType HitType = DetermineHitType(OtherActor, SweepResult);
	
	// 시커에 대한 데미지 처리
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (Seeker && OtherComp == Seeker->GetMesh() && OwningTrap)
	{
		// 시커에게만 데미지 적용
		if (OwningTrap)
		{
			OwningTrap->HandleTrapDamage(OtherActor);
		}
		
		// 히트 효과 처리
		Multicast_PlayHitEffects(HitType, SweepResult.ImpactPoint, SweepResult.ImpactNormal);
		
		// 화살 박히기 처리
		StickWithVisualOnly(SweepResult);
		return;
	}

	// 다른 몬스터 (Pawn이지만 시커가 아닌 경우)
	if (OtherActor->IsA<APawn>() && !Seeker)
	{
		// 거리 체크 후 VFX 재생
		if (ShouldPlayEffectsForDistance(SweepResult.ImpactPoint))
		{
			Multicast_PlayHitVFXOnly(SweepResult.ImpactPoint, SweepResult.ImpactNormal);
		}
		
		// 최적화된 풀 반환
		ReturnToPoolOptimized();
		return;
	}

	// 벽이나 구조물 처리
	if (HitType == EArrowHitType::Wall)
	{
		// 히트 효과 처리
		Multicast_PlayHitEffects(HitType, SweepResult.ImpactPoint, SweepResult.ImpactNormal);
		
		// 화살 박히기 처리
		StickWithVisualOnly(SweepResult);
		return;
	}
}

EArrowHitType AGS_ArrowTrapProjectile::DetermineHitType(AActor* HitActor, const FHitResult& Hit) const
{
	if (!HitActor)
	{
		return EArrowHitType::Other;
	}

	// 시커 플레이어 체크
	if (HitActor->IsA<AGS_Seeker>())
	{
		return EArrowHitType::Player;
	}

	// 스태틱 메시 컴포넌트(벽) 체크
	if (Hit.GetComponent() && Hit.GetComponent()->IsA<UStaticMeshComponent>())
	{
		return EArrowHitType::Wall;
	}

	// 일반적인 벽이나 구조물 체크 (Pawn이 아닌 것들)
	if (!HitActor->IsA<APawn>())
	{
		return EArrowHitType::Wall;
	}

	return EArrowHitType::Other;
}

void AGS_ArrowTrapProjectile::HandleHitEffects(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	// 사운드 재생
	PlayHitSound(HitType, ImpactPoint);
	
	// VFX 재생
	PlayHitVFX(HitType, ImpactPoint, ImpactNormal);
}

void AGS_ArrowTrapProjectile::PlayHitSound(EArrowHitType HitType, const FVector& Location)
{
	UAkAudioEvent* SoundToPlay = nullptr;

	switch (HitType)
	{
		case EArrowHitType::Player:
			SoundToPlay = PlayerHitSoundEvent;
			break;
		case EArrowHitType::Wall:
		case EArrowHitType::Other:
		default:
			SoundToPlay = ImpactSoundEvent;
			break;
	}

	if (SoundToPlay)
	{
		UAkGameplayStatics::PostEventAtLocation(SoundToPlay, Location, FRotator::ZeroRotator, GetWorld());
	}
}

void AGS_ArrowTrapProjectile::PlayHitVFX(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	UNiagaraSystem* VFXToPlay = nullptr;

	switch (HitType)
	{
		case EArrowHitType::Player:
			VFXToPlay = PlayerHitVFX;
			break;
		case EArrowHitType::Wall:
		case EArrowHitType::Other:
		default:
			VFXToPlay = ImpactVFX;
			break;
	}

	if (VFXToPlay)
	{
		// 히트 노말을 기준으로 회전 계산
		FRotator VFXRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			VFXToPlay, 
			ImpactPoint, 
			VFXRotation
		);
	}
}

void AGS_ArrowTrapProjectile::Multicast_PlayHitEffects_Implementation(EArrowHitType HitType, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	HandleHitEffects(HitType, ImpactPoint, ImpactNormal);
}

void AGS_ArrowTrapProjectile::Multicast_PlayHitVFXOnly_Implementation(const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	// 임팩트 VFX만 재생 (사운드 없음)
	if (ImpactVFX)
	{
		FRotator VFXRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			ImpactVFX, 
			ImpactPoint, 
			VFXRotation
		);
	}
}

void AGS_ArrowTrapProjectile::StickWithVisualOnly(const FHitResult& Hit)
{
	if (!ProjectileMesh)
	{
		return;
	}

	// 화살이 박힌 위치
	FVector SpawnLocation = Hit.ImpactPoint;

	FVector ArrowDirection = GetVelocity().GetSafeNormal();
	FRotator SpawnRotation = FRotationMatrix::MakeFromX(ArrowDirection).Rotator();

	FRotator AdjustedRotation = SpawnRotation + FRotator(0.f, -90.f, 0.f);

	// 약간 박히게 밀어넣기
	SpawnLocation = Hit.ImpactPoint - ArrowDirection * 1.f;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	

	AGS_TrapVisualProjectile* VisualArrow = GetWorld()->SpawnActor<AGS_TrapVisualProjectile>(AGS_TrapVisualProjectile::StaticClass(), SpawnLocation, AdjustedRotation, Params);
	
	if (!VisualArrow)
	{
		return;
	}
	
	if (VisualArrow)
	{
		VisualArrow->AttachToComponent(Hit.GetComponent(), FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);

	}

	if (OwningPool)
	{
		OwningPool->ReturnProjectile(this);
	}
}

bool AGS_ArrowTrapProjectile::IsReady() const
{
	//숨겨져 있으면 true를 반환 
	return IsHidden() && !IsPendingKillPending();
}

void AGS_ArrowTrapProjectile::ActivateProjectile(const FVector& SpawnLocation, const FRotator& Rotation, float Speed)
{
	// 기본 null 체크 (IsValid 사용하지 않음)
	if (!this)
	{
		return;
	}

	// 컴포넌트 null 체크 먼저 수행
	if (!ProjectileMovementComponent)
	{
		return;
	}

	if (!CollisionComponent)
	{
		return;
	}

	// 월드 유효성 체크
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 액터 상태 초기화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false);
	
	// 프로젝타일 무브먼트 컴포넌트 초기화
	ProjectileMovementComponent->StopMovementImmediately();
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	ProjectileMovementComponent->UpdateComponentVelocity();

	// 위치와 회전 설정
	SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(Rotation);
	
	// 속도 설정 및 활성화
	const FVector Direction = Rotation.Vector();
	ProjectileMovementComponent->Velocity = Direction * Speed;
	ProjectileMovementComponent->Activate();

	// 콜리전 활성화
	SetActorEnableCollision(true);
	SetLifeSpan(4.0f);
	
	// 화살 효과 관련 blueprint native event
	OnActivateEffect();
}

void AGS_ArrowTrapProjectile::DeactivateProjectile()
{
	// 기본 null 체크
	if (!this || !ProjectileMovementComponent)
	{
		return;
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 프로젝타일 무브먼트 컴포넌트 비활성화
	ProjectileMovementComponent->Deactivate();
	ProjectileMovementComponent->StopMovementImmediately();
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	ProjectileMovementComponent->UpdateComponentVelocity();
}

void AGS_ArrowTrapProjectile::OnActivateEffect_Implementation()
{

}

void AGS_ArrowTrapProjectile::ReturnToPoolOptimized()
{
	// 최적화된 풀 반환 로직
	if (OwningPool)
	{
		// 즉시 비활성화
		DeactivateProjectile();
		
		// 풀로 반환
		OwningPool->ReturnProjectile(this);
	}
	else
	{
		// 풀이 없으면 일반적인 소멸 처리
		SetLifeSpan(0.1f);
	}
}

bool AGS_ArrowTrapProjectile::ShouldPlayEffectsForDistance(const FVector& ImpactPoint) const
{
	// 거리 컬링이 비활성화되어 있으면 항상 재생
	if (!bUseDistanceCulling)
	{
		return true;
	}
	
	// 플레이어와의 거리 체크
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* PlayerPawn = PC->GetPawn())
			{
				float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), ImpactPoint);
				return Distance <= MaxEffectDistance;
			}
		}
	}
	
	// 플레이어를 찾을 수 없으면 기본적으로 재생
	return true;
}
