#include "Props/Trap/GS_TrapBase.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Engine/DamageEvents.h"
#include "Props/Trap/TrapMotion/GS_TrapMotionCompBase.h"
#include "EngineUtils.h"
#include "System/GameMode/GS_InGameGM.h"
#include "Character/F_GS_DamageEvent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"
#include "AkComponent.h"
#include "AkAudioDevice.h"
#include "VFX/GS_VFX_FunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"

AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;
	RootSceneComp->PrimaryComponentTick.bCanEverTick = false;
	RootSceneComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	RootSceneComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	RotationSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RotationScene"));
	RotationSceneComp->SetupAttachment(RootComponent);
	RotationSceneComp->PrimaryComponentTick.bCanEverTick = false;
	RotationSceneComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	RotationSceneComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	MeshParentSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("MeshParentSceneComp"));
	MeshParentSceneComp->SetupAttachment(RotationSceneComp);
	MeshParentSceneComp->PrimaryComponentTick.bCanEverTick = false;
	MeshParentSceneComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	MeshParentSceneComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	ActivateSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("ActivateSphereComp"));
    ActivateSphereComp->SetupAttachment(MeshParentSceneComp);
	ActivateSphereComp->PrimaryComponentTick.bCanEverTick = false;
	ActivateSphereComp->PrimaryComponentTick.bStartWithTickEnabled = false;
	ActivateSphereComp->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	ActivateSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivateSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivateSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DamageBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBoxComp->SetupAttachment(MeshParentSceneComp);
	// DamageBox 콜리전 설정
	DamageBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//ECC_GameTraceChannel4 : Trap 전용 콜리전
	DamageBoxComp->SetCollisionObjectType(ECC_GameTraceChannel4);
	DamageBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//"OptimizedCollision" 태그가 있는 경우, 플레이어가 근접한 경우에만 콜리전 활성화됨
    DamageBoxComp->ComponentTags.Add("OptimizedCollision");

    AudioAnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AudioAnchor"));
    AudioAnchorComponent->SetupAttachment(RootComponent);
    AudioAnchorComponent->PrimaryComponentTick.bCanEverTick = false;
    AudioAnchorComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
    AudioAnchorComponent->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
    AudioAnchorComponent->SetRelativeLocation(AudioAnchorRelativeLocation);

	// AkComponent는 기본적으로 생성하지 않음 (BP에서 선택적으로 추가)
	TrapAkComponent = nullptr;
}

void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();

    LoadTrapData();

    // TrapData가 로드된 후 AudioAnchor 위치 조정
    AdjustAudioAnchorByPlacement();

    RefreshTrapAudioSetup(true);

	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->ComponentHasTag("OptimizedCollision"))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}


		}
	}
	
	/*if (HasAuthority())
	{
		AGS_TrapManager* TrapManager = GetTrapManager();
		if(TrapManager)
		{
			TrapManager->RegisterTrap(this);
			UE_LOG(LogTemp, Warning, TEXT("[TrapBase] TrapManager in BeginPlay"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[TrapBase] TrapManager is not in BeginPlay"));
		}

	}*/

	DamageBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
	ActivateSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnActivSCompBeginOverlap);
}

void AGS_TrapBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Transform 검증 (서버 안정성)
    // FQuat을 FRotator로 변환
    const FRotator TransformRotation = Transform.GetRotation().Rotator();
    if (!UGS_AudioComponentBase::IsTransformValid(Transform.GetLocation(), TransformRotation))
    {
        UE_LOG(LogTemp, Error, TEXT("[TrapBase] Invalid Transform in OnConstruction - Actor: %s"), *GetName());
        return;
    }

    LoadTrapData();

    // TrapData가 로드된 후 AudioAnchor 위치 조정
    AdjustAudioAnchorByPlacement();

    RefreshTrapAudioSetup(true);
}

void AGS_TrapBase::RefreshTrapAudioSetup(bool bForceFindComponent)
{
    // World 유효성 체크 (서버 안정성)
    if (!GetWorld() || !IsValid(this))
    {
        return;
    }

    if (IsValid(AudioAnchorComponent))
    {
        AudioAnchorComponent->SetRelativeLocation(AudioAnchorRelativeLocation);
    }

    if (bForceFindComponent || TrapAkComponent == nullptr)
    {
        TrapAkComponent = FindComponentByClass<UAkComponent>();
    }

    if (!IsValid(TrapAkComponent))
    {
        return;
    }

    TrapAkComponent->SetComponentTickEnabled(false);
    AttachTrapAkComponentToAnchor();
}

void AGS_TrapBase::AttachTrapAkComponentToAnchor()
{
    if (!IsValid(TrapAkComponent))
    {
        return;
    }

    // World 유효성 체크 (서버 안정성)
    if (!GetWorld() || !IsValid(this))
    {
        return;
    }

    // 오클루전 완전 비활성화 (방 모듈에 의한 소리 차단 방지)
    TrapAkComponent->OcclusionRefreshInterval = 0.0f;
    TrapAkComponent->EnableSpotReflectors = false;  // Spot Reflector 비활성화

    // Wwise의 Diffraction 및 Transmission Loss 기능 비활성화
    // (벽/천장을 통과해서도 소리가 들리도록)
    #if WITH_EDITOR
    // 에디터에서만 디버그 로그 출력
    UE_LOG(LogTemp, Verbose, TEXT("[TrapBase] Audio Occlusion disabled for %s"), *GetName());
    #endif

    if (bUseAudioAnchor && IsValid(AudioAnchorComponent))
    {
        const FVector AnchorLocation = AudioAnchorComponent->GetComponentLocation();
        const FRotator AnchorRotation = AudioAnchorComponent->GetComponentRotation();

        // Transform 검증 (NaN/Infinity 체크)
        if (!UGS_AudioComponentBase::IsTransformValid(AnchorLocation, AnchorRotation))
        {
            UE_LOG(LogTemp, Error, TEXT("[TrapBase] Invalid AudioAnchor Transform detected - Actor: %s"), *GetName());
            return;
        }

        TrapAkComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        TrapAkComponent->AttachToComponent(AudioAnchorComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        TrapAkComponent->SetRelativeLocation(FVector::ZeroVector);
        TrapAkComponent->SetRelativeRotation(FRotator::ZeroRotator);
        TrapAkComponent->SetRelativeScale3D(FVector::OneVector);
        TrapAkComponent->SetWorldLocation(AnchorLocation);
        TrapAkComponent->SetWorldRotation(AnchorRotation);
    }
    else if (IsValid(RootComponent))
    {
        TrapAkComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        TrapAkComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    }
}

void AGS_TrapBase::AdjustAudioAnchorByPlacement()
{
    if (!IsValid(AudioAnchorComponent))
    {
        return;
    }

    // TrapData의 Placement 타입에 따라 AudioAnchor 위치 동적 조정
    switch (TrapData.Placement)
    {
    case ETrapPlacement::Ceiling:
        // 천장 함정: 아래쪽으로 위치 이동 (천장 안쪽에 들어가지 않도록)
        AudioAnchorRelativeLocation = FVector(0.0f, 0.0f, -150.0f); // 1.5m 아래
        break;

    case ETrapPlacement::Wall:
        // 벽 함정: 앞쪽으로 위치 이동 (벽 안쪽에 들어가지 않도록)
        AudioAnchorRelativeLocation = FVector(150.0f, 0.0f, 0.0f); // 1.5m 앞
        break;

    case ETrapPlacement::Floor:
    default:
        // 바닥 함정: 위쪽 (기본값 유지)
        AudioAnchorRelativeLocation = FVector(0.0f, 0.0f, 120.0f); // 1.2m 위
        break;
    }

    AudioAnchorComponent->SetRelativeLocation(AudioAnchorRelativeLocation);

    // TrapAkComponent가 이미 Attach되어 있으면 다시 Attach
    if (IsValid(TrapAkComponent))
    {
        AttachTrapAkComponentToAnchor();
    }

    UE_LOG(LogTemp, Verbose, TEXT("[TrapBase] AudioAnchor adjusted for %s placement: %s"),
        *UEnum::GetValueAsString(TrapData.Placement), *AudioAnchorRelativeLocation.ToString());
}

//함정 활성화
void AGS_TrapBase::OnActivSCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
		if (Seeker)
		{
			// 재발동 시에도 사운드가 들리도록 함정 활성화 사운드 재생
			PlayActivationSound();

			if (!bIsActivated)
			{
				bIsActivated = true;
				if (!HasAuthority())
				{
					Server_ActivateTrap(OtherActor);
				}
				else
				{
					ActivateTrap(OtherActor);
				}

				if (!GetWorld()->GetTimerManager().IsTimerActive(CheckOverlapTimerHandle))
				{
					StartDeactivateTrapCheck();
				}
			}
		}
	}
}

void AGS_TrapBase::Server_ActivateTrap_Implementation(AActor* TargetActor)
{
	ActivateTrap(TargetActor);
}

void AGS_TrapBase::ActivateTrap_Implementation(AActor* TargetActor)
{
	Multicast_EnableOptimizedCollision();

	// 활성화 사운드 재생
	PlayActivationSound();
}

void AGS_TrapBase::Multicast_EnableOptimizedCollision_Implementation()
{
	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->ComponentHasTag("OptimizedCollision"))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			
		}
	}
}

//Sphere Comp에 End Overlap 시,
void AGS_TrapBase::StartDeactivateTrapCheck()
{
	GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AGS_TrapBase::CheckOverlappingSeeker, 5.0f, true);
}

void AGS_TrapBase::CheckOverlappingSeeker()
{
	TArray<AActor*> OverlappingActors;
	ActivateSphereComp->GetOverlappingActors(OverlappingActors, AGS_Seeker::StaticClass());

	if (OverlappingActors.Num() == 0)
	{
		DeActivateTrap();
		GetWorld()->GetTimerManager().ClearTimer(CheckOverlapTimerHandle);
		bIsActivated = false;
	}
}

void AGS_TrapBase::DeActivateTrap_Implementation()
{
	Multicast_DisableOptimizedCollision();

	// 비활성화 사운드 재생
	PlayDeactivationSound();
}

void AGS_TrapBase::Multicast_DisableOptimizedCollision_Implementation()
{
	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->ComponentHasTag("OptimizedCollision"))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			
		}
	}
}

//함정 데미지 
void AGS_TrapBase::LoadTrapData()
{
	if (!TrapDataTable) return;
	FTrapData* FoundTrapData = TrapDataTable->FindRow<FTrapData>(TrapID, TEXT("LoadTrapData"));
	if (FoundTrapData)
	{
		TrapData = *FoundTrapData;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrapData not found for TrapID : %s"), *TrapID.ToString());
	}
}

//데미지 박스에 오버랩된 경우 HandleTrapDamage 함수 실행
void AGS_TrapBase::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    if (!HasAuthority())
    {
        return;
    }

    if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Overlapped Actor: %s (%s)"), *OtherActor->GetName(), *OtherActor->GetClass()->GetName());
        if (OtherComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("Overlapped Component: %s (%s)"), *OtherComp->GetName(), *OtherComp->GetClass()->GetName());
        }

        // 서버
        DamageBoxEffect(Seeker);
        CustomTrapEffect(Seeker);
        HandleTrapDamage(Seeker);

        // 함정 히트 사운드 재생
        PlayHitSound();
        return;
    }

    if (bPlayHitSoundOnEnvironmentImpact)
    {
        // 환경 충돌 체크 람다 (안정성 강화)
        auto IsEnvironmentHit = [](AActor* HitActor, UPrimitiveComponent* HitComp) -> bool
        {
            // nullptr 체크 강화
            if (!IsValid(HitActor))
            {
                return false;
            }

            if (HitComp && IsValid(HitComp))
            {
                const ECollisionChannel Channel = HitComp->GetCollisionObjectType();
                return Channel == ECC_WorldStatic || Channel == ECC_WorldDynamic;
            }

            // RootComponent 체크
            if (const UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
            {
                if (IsValid(RootPrim))
                {
                    const ECollisionChannel Channel = RootPrim->GetCollisionObjectType();
                    return Channel == ECC_WorldStatic || Channel == ECC_WorldDynamic;
                }
            }

            return false;
        };

        if (IsEnvironmentHit(OtherActor, OtherComp))
        {
            UE_LOG(LogTemp, Verbose, TEXT("[TrapBase] Environment impact detected (%s)"), *GetNameSafe(OtherActor));
            PlayHitSound();
        }
    }
}

void AGS_TrapBase::Server_HandleTrapDamage_Implementation(AActor* OtherActor)
{
	HandleTrapDamage(OtherActor);
}

EHitReactType AGS_TrapBase::GetHitReactType() const
{
	return EHitReactType::Interrupt;
}

void AGS_TrapBase::HandleTrapDamage(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Trap: %s / Class: %s / HitReactType: %s"),
		*GetName(),
		*GetClass()->GetName(),
		*UEnum::GetValueAsString(GetHitReactType()));
	if (!OtherActor) return;
	AGS_Seeker* DamagedSeeker = Cast<AGS_Seeker>(OtherActor);
	if (!DamagedSeeker) return;

	//디버프 연결
	if (UGS_DebuffComp* DebuffComp = DamagedSeeker->FindComponentByClass<UGS_DebuffComp>())
	{

		const FTrapEffect& Effect = TrapData.Effect;

		//Stun
		if (Effect.bStun)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Stun, nullptr);
		}

		//Slow
		if (Effect.bSlow)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Slow, nullptr);

		}

		//Burn
		if (Effect.bBurn)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Burn, nullptr);

		}

		//Lava
		if (Effect.bLava)
		{
			DebuffComp->ApplyDebuff(EDebuffType::Lava, nullptr);

		}
	}

	//기본 데미지 부여
	if (TrapData.Effect.Damage <= 0.f) return;
	
	FGS_DamageEvent DamageEvent;
	DamageEvent.HitReactType = GetHitReactType();

	DamagedSeeker->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);

	// 혈흔 이펙트 재생 (시커의 메시 위치에서)
	if (USkeletalMeshComponent* SeekerMesh = DamagedSeeker->GetMesh())
	{
		// 메시의 중앙 위치 가져오기 (Pelvis 본 또는 루트 본)
		FVector HitLocation = SeekerMesh->GetSocketLocation(FName("pelvis"));
		if (HitLocation.IsNearlyZero())
		{
			HitLocation = SeekerMesh->GetComponentLocation();
		}
		
		Multicast_PlayTrapHitBloodEffect(HitLocation);
	}

}

void AGS_TrapBase::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{

}

void AGS_TrapBase::Server_DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Server_DamageBoxEffect_Implementation called"));
	DamageBoxEffect(OtherActor);
}


void AGS_TrapBase::Multicast_DamageBoxEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_DamageBoxEffect_Implementation called"));
	DamageBoxEffect(TargetActor);
}

void AGS_TrapBase::DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("DamageBoxEffect Applied"));
}


void AGS_TrapBase::Server_CustomTrapEffect_Implementation(AActor* TargetActor)
{
	CustomTrapEffect(TargetActor);
}

void AGS_TrapBase::CustomTrapEffect_Implementation(AActor* TargetActor)
{

}

void AGS_TrapBase::Multicast_PlayTrapHitBloodEffect_Implementation(FVector HitLocation)
{
	// 함정 데이터에서 혈흔 이펙트 가져오기 (개별 함정에서 오버라이드 가능)
	UNiagaraSystem* BloodEffectToUse = TrapData.TrapHitBloodEffect;

	// 혈흔 이펙트 재생
	UGS_VFX_FunctionLibrary::PlayBloodEffect(this, BloodEffectToUse, HitLocation, FRotator::ZeroRotator, 1.0f);
}

//플레이어가 안에 있는 경우 밀쳐내는 함수
void AGS_TrapBase::PushCharacterInBox(UBoxComponent* CollisionBox, float PushPower)
{
	if (!CollisionBox) return;

	TArray<AActor*> OverlappingActors;
	CollisionBox->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		AGS_Character* Character = Cast<AGS_Character>(Actor);
		if (Character)
		{
			FVector LocalCharacterLocation = GetActorTransform().InverseTransformPosition(Character->GetActorLocation());
			FVector PushDirection = (LocalCharacterLocation.Y >= 0.0f)
				? GetActorRightVector()
				: -GetActorRightVector();

			if (IsBlockedInDirection(Character->GetActorLocation(), PushDirection, 100.0f, Character))
			{
				PushDirection *= -1.0f;
			}

			PushDirection.Z = 0.0f;
			PushDirection = PushDirection.GetSafeNormal();

			FVector LaunchVelocity = PushDirection * PushPower + FVector(0, 0, 200.0f);

			Character->LaunchCharacter(LaunchVelocity, true, true);
		}
	}

}

bool AGS_TrapBase::IsBlockedInDirection(const FVector& Start, const FVector& Direction, float Distance,  AGS_Character* CharacterToIgnore)
{
	FHitResult HitResult;
	FVector End = Start + Direction * Distance;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (CharacterToIgnore)
	{
		Params.AddIgnoredActor(CharacterToIgnore);
	}

	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params);
}

//Trap Motion
AGS_TrapManager* AGS_TrapBase::GetTrapManager() const
{
	/*if (UWorld* World = GetWorld())
	{
		if (AGS_InGameGM* GM = Cast<AGS_InGameGM>(World->GetAuthGameMode()))
		{
			return GM->GetTrapManager();
		}
	}
	return nullptr;*/

	for (TActorIterator<AGS_TrapManager> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

UGS_TrapMotionCompBase* AGS_TrapBase::GetValidMotionComponent() const
{
	TArray<UActorComponent*> Components;
	GetComponents(Components);

	bool bFoundAnyMotionComp = false;

	for (UActorComponent* Comp : Components)
	{
		if (UGS_TrapMotionCompBase* MotionComp = Cast<UGS_TrapMotionCompBase>(Comp))
		{
			/*UE_LOG(LogTemp, Warning, TEXT("[Trap: %s] MotionComp exists : %s / Active: %s"),
				*GetName(), *MotionComp->GetName(), MotionComp->IsActive() ? TEXT("True") : TEXT("False"));*/
				return MotionComp;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("[Trap: %s] MotionComp does not exist at all"), *GetName());
	return nullptr;
}

bool AGS_TrapBase::CanStartMotion() const
{
	return true;
}

//void AGS_TrapBase::ClearDotTimerForActor(AActor* Actor)
//{
//	if (!Actor)
//	{
//		return;
//	}
//	FTimerHandle TimerHandle;
//	if (ActiveDoTTimers.RemoveAndCopyValue(Actor, TimerHandle))
//	{
//		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
//		UE_LOG(LogTemp, Warning, TEXT("DoT Timer Successfully ended - Actor: %s"), *GetNameSafe(Actor));
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("DoT Timer failed to end - Actor: %s (no timer)"), *GetNameSafe(Actor));
//	}
//
//}

//void AGS_TrapBase::ApplyDotDamage(AActor* DamagedActor)
//{
//	if (!DamagedActor || !HasAuthority() || !TrapData.Effect.bDoT)
//	{
//		return;
//	}
//
//	if (ActiveDoTTimers.Contains(DamagedActor))
//	{
//		ClearDotTimerForActor(DamagedActor);
//	}
//
//	int32 CurrentTick = 0;
//	FTimerHandle TimerHandle;
//
//	TWeakObjectPtr<AActor> WeakActor = DamagedActor;
//
//	FTimerDelegate Delegate;
//	//타이머가 끝나면 실행되는 람다
//	Delegate.BindLambda([=, this]() mutable
//		{
//			if (!WeakActor.IsValid() || !TrapData.Effect.bDoT)
//			{
//				ClearDotTimerForActor(WeakActor.Get());
//				return;
//			}
//			
//			AActor* ValidActor = WeakActor.Get();
//
//			if (!ValidActor || !IsValid(ValidActor))
//			{
//				ClearDotTimerForActor(ValidActor);
//				return;
//			}
//			FDamageEvent DamageEvent;
//			ValidActor->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);
//			UE_LOG(LogTemp, Warning, TEXT("CurrentTick : %d"), CurrentTick);
//			CurrentTick++;
//
//
//
//			if (CurrentTick >= TrapData.Effect.DamageCount)
//			{
//				////current tick이 damage count보다 같거나 크다면 타이머 초기화 후 ActiveDoTTimers 맵에서 제거
//				//if (ActiveDoTTimers.Contains(ValidActor))
//				//{
//				//	GetWorld()->GetTimerManager().ClearTimer(ActiveDoTTimers[ValidActor]);
//				//	
//				//	//크래시 지점
//				//	ActiveDoTTimers.Remove(ValidActor);
//				//	//
//				//}
//
//				ClearDotTimerForActor(ValidActor);
//			}
//		});
//
//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, TrapData.Effect.DamageInterval, true);
//	ActiveDoTTimers.Add(DamagedActor, TimerHandle);
//}

// ===================
// Audio Functions Implementation
// ===================

bool AGS_TrapBase::IsRTSMode() const
{
	if (!GetWorld())
	{
		return false;
	}

	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC)
	{
		return false;
	}

	return Cast<AGS_RTSController>(LocalPC) != nullptr;
}

UAkAudioEvent* AGS_TrapBase::SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound) const
{
	const bool bRTS = IsRTSMode();
	return bRTS ? RTSSound : TPSSound;
}

bool AGS_TrapBase::ShouldPlayTrapSoundAtLocation(const FVector& TrapLocation) const
{
	// 월드 유효성 체크
	if (!GetWorld() || !IsValid(this))
	{
		return false;
	}

	// 위치 검증 (NaN/Infinity 체크)
	if (!UGS_AudioComponentBase::IsLocationValid(TrapLocation))
	{
		return false;
	}

	// 플레이어 컨트롤러 가져오기
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC || !IsValid(LocalPC))
	{
		return false;
	}

	// 리스너 위치와 카메라 회전 정보 가져오기
	FVector ListenerLocation;
	FRotator CameraRotation;
	
	if (LocalPC->PlayerCameraManager && IsValid(LocalPC->PlayerCameraManager))
	{
		ListenerLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
		CameraRotation = LocalPC->PlayerCameraManager->GetCameraRotation();
	}
	else if (APawn* PlayerPawn = LocalPC->GetPawn())
	{
		if (IsValid(PlayerPawn))
		{
			ListenerLocation = PlayerPawn->GetActorLocation();
			CameraRotation = PlayerPawn->GetActorRotation();
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	// 리스너 위치 검증
	if (!UGS_AudioComponentBase::IsLocationValid(ListenerLocation))
	{
		return false;
	}

	const float DistanceToListener = FVector::Dist(TrapLocation, ListenerLocation);

	// 매우 가까우면 항상 재생 (모든 방향에서)
	if (DistanceToListener <= 800.0f) // 8m
	{
		return true;
	}

	// 모드별 거리 및 방향 체크
	const bool bRTS = IsRTSMode();

	if (bRTS)
	{
		// RTS 모드: 거리만 체크 (200m)
		return DistanceToListener <= 20000.0f;
	}
	else
	{
		// TPS 모드: 카메라 방향 체크 추가
		const FVector CameraForwardVector = CameraRotation.Vector();
		const FVector DirectionToSource = (TrapLocation - ListenerLocation).GetSafeNormal();

		// 카메라 정면 방향과의 내적 (앞/뒤 판단)
		const float DotProduct = FVector::DotProduct(DirectionToSource, CameraForwardVector);

		// 상하 방향 벡터 계산 (천장/바닥 함정 판별)
		const FVector UpVector = FVector::UpVector;
		const float VerticalDot = FVector::DotProduct(DirectionToSource, UpVector);

		// 높이 차이 계산
		const float HeightDifference = TrapLocation.Z - ListenerLocation.Z;
		const float AbsHeightDiff = FMath::Abs(HeightDifference);

		// 수평 거리 계산 (Z축 제외)
		const FVector HorizontalTrapLocation(TrapLocation.X, TrapLocation.Y, ListenerLocation.Z);
		const float HorizontalDistance = FVector::Dist(HorizontalTrapLocation, ListenerLocation);

		// 천장/바닥 함정 판별
		// VerticalDot > 0.5f: 소리가 위쪽에서 옴 (천장 함정)
		// VerticalDot < -0.5f: 소리가 아래쪽에서 옴 (바닥 함정)
		const bool bIsCeilingSound = (VerticalDot > 0.3f) && (HeightDifference > 100.0f); // 1m 이상 위쪽
		const bool bIsFloorSound = (VerticalDot < -0.3f) && (HeightDifference < -100.0f); // 1m 이상 아래쪽
		const bool bIsVerticalSound = bIsCeilingSound || bIsFloorSound;

		// 천장/바닥 함정 특별 처리 (경고/활성화 사운드)
		if (bRelaxDirectionFilterForWarning && bIsVerticalSound)
		{
			// 천장/바닥 함정의 경우 방향 필터링 완화
			// 수평 거리가 가까우면 카메라 방향과 무관하게 들리도록
			if (HorizontalDistance <= 1500.0f) // 수평 거리 15m 이내
			{
				const bool bPlaySound = DistanceToListener <= ActivationSoundMaxDistance;
				#if WITH_EDITOR
				if (!bPlaySound)
				{
					UE_LOG(LogTemp, Verbose, TEXT("[TrapBase] Vertical sound blocked - Distance: %.1f, MaxDistance: %.1f, Height: %.1f, HorizontalDist: %.1f"),
						DistanceToListener, ActivationSoundMaxDistance, HeightDifference, HorizontalDistance);
				}
				#endif
				return bPlaySound;
			}

			// 수평 거리가 멀어도 카메라가 소리 쪽을 향하고 있으면 허용
			if (DotProduct > -0.3f) // 약 107도 범위
			{
				const bool bPlaySound = DistanceToListener <= ActivationSoundMaxDistance;
				return bPlaySound;
			}
		}

		// 일반적인 필터링: 카메라 뒤쪽 필터링 (옆쪽 사각지대 포함)
		if (DotProduct < 0.0f)
		{
			return false; // 카메라 뒤의 소리는 안 들림
		}

		// 카메라 앞쪽이면 거리 체크 (20m)
		return DistanceToListener <= 2000.0f;
	}
}

void AGS_TrapBase::SetTrapAkComponent(UAkComponent* NewAkComponent)
{
	// 유효성 체크 (서버 안정성)
	if (!IsValid(NewAkComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TrapBase] Attempted to set invalid TrapAkComponent - Actor: %s"), *GetName());
		return;
	}

	TrapAkComponent = NewAkComponent;
    RefreshTrapAudioSetup(false);
}

void AGS_TrapBase::PlayActivationSound()
{
	if (!HasAuthority())
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.ActivationSound_TPS, TrapData.ActivationSound_RTS);
	if (SoundEvent)
	{
		Multicast_PlayActivationSound();
	}
}

void AGS_TrapBase::PlayDeactivationSound()
{
	if (!HasAuthority())
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.DeactivationSound_TPS, TrapData.DeactivationSound_RTS);
	if (SoundEvent)
	{
		Multicast_PlayDeactivationSound();
	}
}

void AGS_TrapBase::PlayHitSound()
{
	if (!HasAuthority())
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.HitSound_TPS, TrapData.HitSound_RTS);
	if (SoundEvent)
	{
		Multicast_PlayHitSound();
	}
}

void AGS_TrapBase::Multicast_PlayActivationSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Actor 유효성 체크 (서버 안정성)
	if (!IsValid(this))
	{
		return;
	}

	// 거리 기반 최적화 체크
	if (!ShouldPlayTrapSoundAtLocation(GetActorLocation()))
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.ActivationSound_TPS, TrapData.ActivationSound_RTS);
	if (SoundEvent)
	{
		// TrapAkComponent가 있으면 AudioAnchor 위치에서 재생, 없으면 Actor 자체 사용
		if (IsValid(TrapAkComponent))
		{
			TrapAkComponent->PostAkEvent(SoundEvent, 0, FOnAkPostEventCallback());
		}
		else
		{
			UAkGameplayStatics::PostEvent(SoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}
}

void AGS_TrapBase::Multicast_PlayDeactivationSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Actor 유효성 체크 (서버 안정성)
	if (!IsValid(this))
	{
		return;
	}

	// 거리 기반 최적화 체크
	if (!ShouldPlayTrapSoundAtLocation(GetActorLocation()))
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.DeactivationSound_TPS, TrapData.DeactivationSound_RTS);
	if (SoundEvent)
	{
		// TrapAkComponent가 있으면 AudioAnchor 위치에서 재생, 없으면 Actor 자체 사용
		if (IsValid(TrapAkComponent))
		{
			TrapAkComponent->PostAkEvent(SoundEvent, 0, FOnAkPostEventCallback());
		}
		else
		{
			UAkGameplayStatics::PostEvent(SoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}
}

void AGS_TrapBase::Multicast_PlayHitSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Actor 유효성 체크 (서버 안정성)
	if (!IsValid(this))
	{
		return;
	}

	// 거리 기반 최적화 체크
	if (!ShouldPlayTrapSoundAtLocation(GetActorLocation()))
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.HitSound_TPS, TrapData.HitSound_RTS);
	if (SoundEvent)
	{
		// TrapAkComponent가 있으면 AudioAnchor 위치에서 재생, 없으면 Actor 자체 사용
		if (IsValid(TrapAkComponent))
		{
			TrapAkComponent->PostAkEvent(SoundEvent, 0, FOnAkPostEventCallback());
		}
		else
		{
			UAkGameplayStatics::PostEvent(SoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}
}