#include "Weapon/Projectile/Guardian/GS_DrakharProjectile.h"

#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Guardian/GS_Drakhar.h"

#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/F_GS_DamageEvent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"


AGS_DrakharProjectile::AGS_DrakharProjectile()
{
	IndicatorVFX = nullptr;
	IndicatorComponent = nullptr;
	IndicatorRadius = 250.0f;
	bHasHitTarget = false;
	CachedIndicatorScale = 1.0f; // 이전 스케일 값을 저장할 멤버 변수
}

void AGS_DrakharProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_DrakharProjectile, IndicatorVFX);
	DOREPLIFETIME(AGS_DrakharProjectile, IndicatorRadius);
}

void AGS_DrakharProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 서버 전용 로직
	if (HasAuthority())
	{
		if (GetInstigator() && CollisionComponent)
		{
			CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
		}
	}

	// 인디케이터는 SetIndicatorVFX()가 호출된 후에 생성됨
	// BeginPlay()에서는 생성하지 않음 (타이밍 문제 방지)
}

void AGS_DrakharProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 안전한 타이머 정리 - 레벨 전환 시 크래시 방지
	SafeClearTimer(IndicatorActivateTimerHandle);

	// 인디케이터 정리 (메모리 누수 방지)
	CleanupIndicator();

	Super::EndPlay(EndPlayReason);
}

void AGS_DrakharProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Early return: 레벨 전환 중 체크 및 생명주기 검증
	if (!IsWorldContextValid() || !IsValid(this))
	{
		return;
	}

	// Early return: SoundTrigger 콜리전 무시
	if (OtherComp && OtherComp->GetCollisionProfileName() == FName("SoundTrigger"))
	{
		return;
	}

	// 중복 충돌 방지 (이미 파괴된 상태인지 확인)
	if (bHasHitTarget)
	{
		return;
	}
	bHasHitTarget = true;

	// 충돌 처리
	const bool bHitCharacter = TryApplyDamageToCharacter(OtherActor);

	// 소유자에게 충돌 이벤트 알림 (안전한 호출)
	NotifyOwnerOfImpact(Hit, bHitCharacter);

	// 정리 및 파괴 (딜레이를 두어 이펙트 완료 보장)
	FTimerHandle DestroyTimerHandle;
	UWorld* World = GetWorld();
	if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
	{
		World->GetTimerManager().SetTimer(
			DestroyTimerHandle,
			[this]()
			{
				SafeDestroyProjectile();
			},
			0.1f, // 0.1초 딜레이로 이펙트 완료 보장
			false
		);
	}
	else
	{
		// 월드가 유효하지 않으면 즉시 정리
		SafeDestroyProjectile();
	}
}

// === 캐릭터에게 데미지 적용 시도 ===
bool AGS_DrakharProjectile::TryApplyDamageToCharacter(AActor* HitActor)
{
	// Early return: 유효성 검사
	if (!IsValid(HitActor))
	{
		return false;
	}

	// 캐릭터인지 확인
	AGS_Character* DamagedCharacter = Cast<AGS_Character>(HitActor);
	if (!DamagedCharacter)
	{
		return false;
	}

	// Early return: 가디언은 아군이므로 데미지 적용 안함
	if (DamagedCharacter->IsA<AGS_Guardian>())
	{
		return false;
	}

	// Early return: 소유자 검증
	AActor* ProjectileOwner = GetOwner();
	if (!ProjectileOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DrakharProjectile] TryApplyDamageToCharacter: Owner is NULL!"));
		return false;
	}

	// 데미지 적용
	FGS_DamageEvent DamageEvent;
	DamageEvent.HitReactType = EHitReactType::Interrupt;

	const float DamageAmount = 120.0f; // TODO: 스킬 계수로 변경
	DamagedCharacter->TakeDamage(DamageAmount, DamageEvent, ProjectileOwner->GetInstigatorController(), this);

	return true;
}

// === 소유자에게 충돌 이벤트 알림 ===
void AGS_DrakharProjectile::NotifyOwnerOfImpact(const FHitResult& Hit, bool bHitCharacter)
{
	AGS_Drakhar* OwnerDrakhar = Cast<AGS_Drakhar>(GetOwner());
	if (!OwnerDrakhar)
	{
		return;
	}

	// 충돌 정보 추출 (유효하지 않으면 투사체 정보 사용)
	FVector ImpactLocation = Hit.ImpactPoint;
	if (ImpactLocation.IsZero())
	{
		ImpactLocation = GetActorLocation();
	}

	FVector ImpactNormal = Hit.ImpactNormal;
	if (ImpactNormal.IsZero())
	{
		ImpactNormal = -GetActorForwardVector();
	}

	// Drakhar에게 충돌 이펙트 재생 요청
	OwnerDrakhar->HandleDraconicProjectileImpact(ImpactLocation, ImpactNormal, bHitCharacter);
}

void AGS_DrakharProjectile::SetIndicatorVFX(UNiagaraSystem* InIndicatorVFX, float InIndicatorRadius)
{
	IndicatorVFX = InIndicatorVFX;
	IndicatorRadius = InIndicatorRadius;

	// VFX 설정 완료

	// 서버에서만 VFX가 설정되면 즉시 인디케이터 생성
	// 클라이언트는 OnRep_IndicatorVFX에서 생성
	if (HasAuthority() && IsWorldContextValid())
	{
		SpawnGroundIndicator();
	}
}

void AGS_DrakharProjectile::OnRep_IndicatorVFX()
{
	// 레벨 전환 중에는 VFX 생성하지 않음
	if (!IsWorldContextValid() || !IndicatorVFX)
	{
		return;
	}

	// 클라이언트에서 리플리케이트된 VFX로 인디케이터 생성
	SpawnGroundIndicator();
}

// === 투사체 궤적 예측하여 충돌 지점 반환 ===
bool AGS_DrakharProjectile::PredictProjectileImpactLocation(FVector& OutImpactLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UProjectileMovementComponent* ProjectileMovement = GetProjectileMovement();
	if (!ProjectileMovement)
	{
		UE_LOG(LogTemp, Error, TEXT("[DrakharProjectile] PredictProjectileImpactLocation: ProjectileMovement is NULL!"));
		return false;
	}

	// 투사체 경로 예측 시작

	// 궤적 예측 파라미터 설정
	FPredictProjectilePathParams PredictParams;
	PredictParams.StartLocation = GetActorLocation();
	PredictParams.LaunchVelocity = ProjectileMovement->Velocity;
	PredictParams.bTraceWithCollision = true;
	PredictParams.ProjectileRadius = CollisionComponent ? CollisionComponent->GetScaledSphereRadius() : 10.0f;
	PredictParams.MaxSimTime = MaxPredictionTime;
	PredictParams.bTraceWithChannel = true;
	PredictParams.TraceChannel = ECC_Visibility;
	PredictParams.SimFrequency = SimulationFrequency;
	PredictParams.OverrideGravityZ = World->GetGravityZ();

	// 무시할 액터 설정
	PredictParams.ActorsToIgnore.Add(this);
	if (GetInstigator())
	{
		PredictParams.ActorsToIgnore.Add(GetInstigator());
	}
	if (GetOwner())
	{
		PredictParams.ActorsToIgnore.Add(GetOwner());
	}

	// 디버그 시각화
	PredictParams.DrawDebugType = bShowProjectilePath ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	PredictParams.DrawDebugTime = DebugTraceDisplayTime;

	// 궤적 예측 실행
	FPredictProjectilePathResult PredictResult;
	const bool bHit = UGameplayStatics::PredictProjectilePath(World, PredictParams, PredictResult);

	if (bHit && PredictResult.HitResult.bBlockingHit)
	{
		OutImpactLocation = PredictResult.HitResult.ImpactPoint;
		// 충돌 위치 예측 완료
		return true;
	}

	// 예측 실패 시 현재 위치 사용
	OutImpactLocation = GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("[DrakharProjectile] Path prediction failed, using actor location."));
	return false;
}

// === 특정 지점에서 지면을 찾아 위치 반환 (성능 최적화) ===
bool AGS_DrakharProjectile::FindGroundLocation(const FVector& TraceStartPoint, FVector& OutGroundLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OutGroundLocation = FVector(TraceStartPoint.X, TraceStartPoint.Y, FallbackGroundZPosition);
		return false;
	}

	// 빠른 실패 체크를 위한 조기 반환
	if (!IsWorldContextValid())
	{
		OutGroundLocation = FVector(TraceStartPoint.X, TraceStartPoint.Y, FallbackGroundZPosition);
		return false;
	}

	// 트레이스 시작/끝 지점 설정 (캐싱으로 최적화)
	const FVector GroundTraceStart = FVector(TraceStartPoint.X, TraceStartPoint.Y, TraceStartPoint.Z + GroundTraceUpOffset);
	const FVector GroundTraceEnd = FVector(TraceStartPoint.X, TraceStartPoint.Y, TraceStartPoint.Z - GroundTraceDownOffset);

	// 쿼리 파라미터 설정 (멀티플레이 안전성을 위해 매번 생성)
	FCollisionQueryParams GroundParams;
	GroundParams.AddIgnoredActor(this);

	// 동적 무시 액터 추가 (변경될 수 있으므로 매번 체크)
	if (GetInstigator())
	{
		GroundParams.AddIgnoredActor(GetInstigator());
	}
	if (GetOwner())
	{
		GroundParams.AddIgnoredActor(GetOwner());
	}

	// 투사체의 모든 컴포넌트 무시 (멀티플레이 환경에서 안전하게 매번 생성)
	TArray<UPrimitiveComponent*> ProjectileComponents;
	GetComponents<UPrimitiveComponent>(ProjectileComponents);
	for (UPrimitiveComponent* Component : ProjectileComponents)
	{
		if (Component)
		{
			GroundParams.AddIgnoredComponent(Component);
		}
	}

	FHitResult GroundHit;

	// 1차 시도: WorldStatic 오브젝트만 감지 (가장 일반적인 경우)
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	if (World->LineTraceSingleByObjectType(GroundHit, GroundTraceStart, GroundTraceEnd, ObjectParams, GroundParams))
	{
		OutGroundLocation = GroundHit.ImpactPoint;
		return true;
	}

	// 2차 시도: WorldDynamic도 포함 (덜 일반적인 경우)
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	if (World->LineTraceSingleByObjectType(GroundHit, GroundTraceStart, GroundTraceEnd, ObjectParams, GroundParams))
	{
		OutGroundLocation = GroundHit.ImpactPoint;
		return true;
	}

	// 실패 시 Fallback 위치 사용
	OutGroundLocation = FVector(TraceStartPoint.X, TraceStartPoint.Y, FallbackGroundZPosition);
	return false;
}

// === 인디케이터 나이아가라 컴포넌트 생성 및 설정 (성능 최적화) ===
void AGS_DrakharProjectile::CreateAndConfigureIndicator(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World || !IndicatorVFX || !IsWorldContextValid())
	{
		return;
	}

	// 이미 생성된 인디케이터가 있는지 확인 (중복 생성 방지)
	if (IndicatorComponent && IsValid(IndicatorComponent) && !IndicatorComponent->IsBeingDestroyed())
	{
		// 기존 인디케이터 위치 업데이트 (멀티플레이 안전성 보장)
		FVector CurrentLocation = IndicatorComponent->GetComponentLocation();
		if (!CurrentLocation.Equals(Location, 1.0f)) // 위치 비교 오차 허용
		{
			IndicatorComponent->SetWorldLocation(Location);
		}

		// 스케일 업데이트 (반경이 변경되었을 가능성)
		const float CurrentScale = IndicatorRadius / DefaultIndicatorRadius;

		// 이전 스케일 값과 비교 (캐싱된 값 사용)
		if (!FMath::IsNearlyEqual(CurrentScale, CachedIndicatorScale))
		{
			IndicatorComponent->SetVectorParameter(FName("Scale_All"), FVector(CurrentScale, CurrentScale, CurrentScale));
			IndicatorComponent->SetFloatParameter(FName("CurrentScale"), CurrentScale);
			CachedIndicatorScale = CurrentScale; // 캐시 업데이트
		}
		return;
	}

	// 나이아가라 시스템 생성 (초기 비활성화 상태)
	IndicatorComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		IndicatorVFX,
		Location,
		FRotator::ZeroRotator,
		FVector(1.0f, 1.0f, 1.0f),
		true,  // bAutoDestroy
		false, // bAutoActivate - 반짝거림 방지
		ENCPoolMethod::None,
		true   // bPreCullCheck
	);

	if (!IndicatorComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[DrakharProjectile] Failed to spawn indicator component!"));
		return;
	}

	// 스케일 계산 및 적용 (현재 스케일도 저장)
	const float Scale = IndicatorRadius / DefaultIndicatorRadius;
	IndicatorComponent->SetVectorParameter(FName("Scale_All"), FVector(Scale, Scale, Scale));
	IndicatorComponent->SetFloatParameter(FName("CurrentScale"), Scale);
	CachedIndicatorScale = Scale; // 캐시 초기화

	// 나이아가라 파라미터 설정 (캐싱으로 최적화)
	static const FName SpawnDelayName = FName("SpawnDelay");
	static const FName InitialAlphaName = FName("InitialAlpha");

	IndicatorComponent->SetFloatParameter(SpawnDelayName, IndicatorSpawnDelay);
	IndicatorComponent->SetFloatParameter(InitialAlphaName, 0.0f);

	// 투사체와 함께 관리하기 위해 AutoDestroy 비활성화
	IndicatorComponent->SetAutoDestroy(false);

	// 인디케이터 생성 완료
}

// === 딜레이 후 인디케이터 활성화 스케줄링 ===
void AGS_DrakharProjectile::ScheduleIndicatorActivation()
{
	if (!IndicatorComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 약한 참조로 크래시 방지
	TWeakObjectPtr<UNiagaraComponent> WeakIndicator = IndicatorComponent;

	World->GetTimerManager().SetTimer(
		IndicatorActivateTimerHandle,
		[WeakIndicator]()
		{
			if (WeakIndicator.IsValid() && !WeakIndicator->IsBeingDestroyed())
			{
				WeakIndicator->Activate(true);
			}
		},
		IndicatorActivationDelay,
		false
	);
}

// === 지면에 인디케이터 생성 ===
void AGS_DrakharProjectile::SpawnGroundIndicator()
{
	// Early return: 월드 검증 및 생명주기 체크
	if (!IsWorldContextValid() || !IsValid(this))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Early return: 데디케이티드 서버에서는 VFX 불필요
	if (World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Early return: 인디케이터 VFX 검증
	if (!IndicatorVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DrakharProjectile] SpawnGroundIndicator: IndicatorVFX is NULL!"));
		return;
	}

	// 1단계: 투사체 충돌 지점 예측
	FVector ImpactLocation;
	PredictProjectileImpactLocation(ImpactLocation);

	// 2단계: 지면 위치 찾기
	FVector GroundLocation;
	FindGroundLocation(ImpactLocation, GroundLocation);

	// 3단계: 인디케이터 생성 및 설정
	CreateAndConfigureIndicator(GroundLocation);

	// 4단계: 딜레이 후 활성화
	ScheduleIndicatorActivation();
}

// ===== 헬퍼 함수 구현 =====

void AGS_DrakharProjectile::CleanupIndicator()
{
	// 타이머 정리
	SafeClearTimer(IndicatorActivateTimerHandle);

	// 나이아가라 컴포넌트 정리
	if (IndicatorComponent && IsValid(IndicatorComponent) && !IndicatorComponent->IsBeingDestroyed())
	{
		// 먼저 비활성화하여 부드러운 종료 (메모리 해제 보장)
		IndicatorComponent->DeactivateImmediate();

		// 시스템이 완전히 정지할 때까지 대기 후 파괴
		FTimerHandle CleanupTimerHandle;
		UWorld* World = GetWorld();
		if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			// 약한 참조를 통한 안전한 캡처
			TWeakObjectPtr<AGS_DrakharProjectile> WeakThis(this);
			World->GetTimerManager().SetTimer(
				CleanupTimerHandle,
				[WeakThis]()
				{
					AGS_DrakharProjectile* StrongThis = WeakThis.Get();
					if (StrongThis && StrongThis->IndicatorComponent &&
						IsValid(StrongThis->IndicatorComponent) &&
						!StrongThis->IndicatorComponent->IsBeingDestroyed())
					{
						StrongThis->IndicatorComponent->DestroyComponent();
						StrongThis->IndicatorComponent = nullptr;
					}
				},
				0.1f, // 0.1초 후 정리
				false
			);
		}
		else
		{
			// 월드가 유효하지 않으면 즉시 정리
			if (IndicatorComponent && IsValid(IndicatorComponent) && !IndicatorComponent->IsBeingDestroyed())
			{
				IndicatorComponent->DestroyComponent();
			}
			IndicatorComponent = nullptr;
		}
	}
	else
	{
		// 이미 정리되었거나 유효하지 않으면 nullptr로 설정
		IndicatorComponent = nullptr;
	}
}

// === 타이머 정리 함수 ===
void AGS_DrakharProjectile::SafeClearTimer(FTimerHandle& TimerHandle)
{
	if (TimerHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (World && World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
		TimerHandle.Invalidate();
	}
}

// === 월드 컨텍스트 검증 함수 ===
bool AGS_DrakharProjectile::IsWorldContextValid() const
{
	UWorld* World = GetWorld();
	return World &&
		   World->IsValidLowLevel() &&
		   !World->bIsTearingDown &&
		   IsValid(World) &&
		   IsValid(this);
}

// === 투사체 파괴 함수 (이펙트 완료 보장) ===
void AGS_DrakharProjectile::SafeDestroyProjectile()
{
	// 추가 안전성 체크 (이미 파괴 중인지 확인)
	if (!IsValid(this) || IsActorBeingDestroyed())
	{
		return;
	}

	// 인디케이터 정리 (메모리 누수 방지)
	CleanupIndicator();

	// 투사체 파괴
	Destroy();
}