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

	// AkComponent는 기본적으로 생성하지 않음 (BP에서 선택적으로 추가)
	TrapAkComponent = nullptr;
}

void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();

	if (!TrapAkComponent)
	{
		TrapAkComponent = FindComponentByClass<UAkComponent>();
	}
	
	if (TrapAkComponent)
	{
		// AkComponent의 틱 비활성화.사운드 재생은 PostEvent를 통해 멀티캐스트로 처리되므로 틱이 필요하지 않음.
		TrapAkComponent->SetComponentTickEnabled(false);
	}

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

	LoadTrapData();
	DamageBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
	ActivateSphereComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnActivSCompBeginOverlap);
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

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (!Seeker ||!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Overlapped Actor: %s (%s)"), *OtherActor->GetName(), *OtherActor->GetClass()->GetName());
	if (OtherComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlapped Component: %s (%s)"), *OtherComp->GetName(), *OtherComp->GetClass()->GetName());
	}

	//서버
	DamageBoxEffect(Seeker);
	CustomTrapEffect(Seeker);
	HandleTrapDamage(Seeker);

	// 함정 히트 사운드 재생
	PlayHitSound();

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
	if (!GetWorld())
	{
		return false;
	}

	// 플레이어 컨트롤러 가져오기
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC)
	{
		return false;
	}

	// 리스너 위치 계산
	FVector ListenerLocation;
	if (LocalPC->PlayerCameraManager)
	{
		ListenerLocation = LocalPC->PlayerCameraManager->GetCameraLocation();
	}
	else if (APawn* PlayerPawn = LocalPC->GetPawn())
	{
		ListenerLocation = PlayerPawn->GetActorLocation();
	}
	else
	{
		return false;
	}

	// 모드별 거리 체크
	const bool bRTS = IsRTSMode();
	const float MaxDistance = bRTS ? 20000.0f : 2000.0f;  // RTS: 200m, TPS: 20m
	const float DistanceToListener = FVector::Dist(TrapLocation, ListenerLocation);

	if (bRTS)
	{
		// RTS 모드: 거리 체크만 (시야각 체크는 너무 복잡하므로 생략)
		return DistanceToListener <= MaxDistance;
	}
	else
	{
		// TPS 모드: 기존 거리 기반 체크
		return DistanceToListener <= MaxDistance;
	}
}

void AGS_TrapBase::SetTrapAkComponent(UAkComponent* NewAkComponent)
{
	TrapAkComponent = NewAkComponent;
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

	// 거리 기반 최적화 체크
	if (!ShouldPlayTrapSoundAtLocation(GetActorLocation()))
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.ActivationSound_TPS, TrapData.ActivationSound_RTS);
	if (SoundEvent)
	{
		// TrapAkComponent가 있으면 해당 컴포넌트를 사용, 없으면 Actor 자체 사용
		AActor* AudioActor = TrapAkComponent ? TrapAkComponent->GetOwner() : this;
		UAkGameplayStatics::PostEvent(SoundEvent, AudioActor, 0, FOnAkPostEventCallback());
	}
}

void AGS_TrapBase::Multicast_PlayDeactivationSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
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
		// TrapAkComponent가 있으면 해당 컴포넌트를 사용, 없으면 Actor 자체 사용
		AActor* AudioActor = TrapAkComponent ? TrapAkComponent->GetOwner() : this;
		UAkGameplayStatics::PostEvent(SoundEvent, AudioActor, 0, FOnAkPostEventCallback());
	}
}

void AGS_TrapBase::Multicast_PlayHitSound_Implementation()
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
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
		// TrapAkComponent가 있으면 해당 컴포넌트를 사용, 없으면 Actor 자체 사용
		AActor* AudioActor = TrapAkComponent ? TrapAkComponent->GetOwner() : this;
		UAkGameplayStatics::PostEvent(SoundEvent, AudioActor, 0, FOnAkPostEventCallback());
	}
}

